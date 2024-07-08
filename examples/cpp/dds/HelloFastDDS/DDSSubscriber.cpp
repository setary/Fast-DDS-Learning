// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file DDSDDSDDSSubscriber.cpp
 *
 */

#include "DDSSubscriber.h"

#include <chrono>
#include <thread>

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>


using namespace eprosima::fastdds::dds;

namespace learning_dds
{

    DDSSubscriber::DDSSubscriber()
        : participant_(nullptr), subscriber_(nullptr), topic_(nullptr), reader_(nullptr), type_(new SimpleInfoPubSubType())
    {
    }

    bool DDSSubscriber::init(
        bool use_env)
    {
        DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
        pqos.name("Participant_sub");
        auto factory = DomainParticipantFactory::get_instance();

        if (use_env)
        {
            factory->load_profiles();
            factory->get_default_participant_qos(pqos);
        }

        participant_ = factory->create_participant(0, pqos);

        if (participant_ == nullptr)
        {
            return false;
        }

        // REGISTER THE TYPE
        type_.register_type(participant_);

        // CREATE THE SUBSCRIBER
        SubscriberQos sqos = SUBSCRIBER_QOS_DEFAULT;

        if (use_env)
        {
            participant_->get_default_subscriber_qos(sqos);
        }

        subscriber_ = participant_->create_subscriber(sqos, nullptr);

        if (subscriber_ == nullptr)
        {
            return false;
        }

        // CREATE THE TOPIC
        TopicQos tqos = TOPIC_QOS_DEFAULT;

        if (use_env)
        {
            participant_->get_default_topic_qos(tqos);
        }

        topic_ = participant_->create_topic(
            "SimpleInfoTopic",
            "SimpleInfo",
            tqos);

        if (topic_ == nullptr)
        {
            return false;
        }

        // CREATE THE READER
        DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
        rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;

        if (use_env)
        {
            subscriber_->get_default_datareader_qos(rqos);
        }

        // reader_ = subscriber_->create_datareader(topic_, rqos, &listener_);
        reader_ = subscriber_->create_datareader_with_profile(topic_, "datareader_profile", &listener_);

        if (reader_ == nullptr)
        {
            return false;
        }

        create_second_subscriber();

        return true;
    }

    void DDSSubscriber::create_second_subscriber()
    {
        DataReader* data_reader =
        subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT);
        if (nullptr == data_reader)
        {
            // Error
            return;
        }

        // Prepare a wait-set to wait for data on the DataReader
        WaitSet wait_set;
        StatusCondition& condition = data_reader->get_statuscondition();
        condition.set_enabled_statuses(StatusMask::data_available());
        wait_set.attach_condition(condition);

        // Create a data and SampleInfo instance
        SimpleInfo data;
        SampleInfo info;

        //Define a timeout of 5 seconds
        eprosima::fastrtps::Duration_t timeout (5, 0);

        while (true)
        {
            ConditionSeq active_conditions;
            if (0 == wait_set.wait(active_conditions, timeout))
            {
                while (0 == data_reader->take_next_sample(&data, &info))
                {
                    if (info.valid_data)
                    {
                        // Do something with the data
                        std::cout << "Received new data value for topic "
                                << topic_->get_name()
                                << std::endl;
                    }
                    else
                    {
                        // If the remote writer is not alive, we exit the reading loop
                        std::cout << "Remote writer for topic "
                                << topic_->get_name()
                                << " is dead" << std::endl;
                        break;
                    }
                }
            }
            else
            {
                std::cout << "No data this time" << std::endl;
            }
        }
    }

    DDSSubscriber::~DDSSubscriber()
    {
        if (reader_ != nullptr)
        {
            subscriber_->delete_datareader(reader_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr)
        {
            participant_->delete_subscriber(subscriber_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    void DDSSubscriber::SubListener::on_subscription_matched(
        DataReader *,
        const SubscriptionMatchedStatus &info)
    {
        if (info.current_count_change == 1)
        {
            matched_ = info.total_count;
            std::cout << "DDSSubscriber matched." << std::endl;
        }
        else if (info.current_count_change == -1)
        {
            matched_ = info.total_count;
            std::cout << "DDSSubscriber unmatched." << std::endl;
        }
        else
        {
            std::cout << info.current_count_change
                      << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
        }
    }

    void DDSSubscriber::SubListener::on_data_available(
        DataReader *reader)
    {
        SampleInfo info;
        if (reader->take_next_sample(&hello_, &info) == ReturnCode_t::RETCODE_OK)
        {
            if (info.instance_state == ALIVE_INSTANCE_STATE)
            {
                samples_++;
                // Print your structure data here.
                std::cout << "Message " << hello_.name() << " " << hello_.id() << " RECEIVED" << std::endl;
            }
        }
    }

    void DDSSubscriber::SubListener::on_requested_incompatible_qos(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::RequestedIncompatibleQosStatus& status)
    {
        std::cout << "on_requested_incompatible_qos" << std::endl;
    }

    void DDSSubscriber::run()
    {
        std::cout << "DDSSubscriber running. Please press enter to stop the DDSSubscriber" << std::endl;
        std::cin.ignore();
    }

    void DDSSubscriber::run(
        uint32_t number)
    {
        std::cout << "DDSSubscriber running until " << number << "samples have been received" << std::endl;
        while (number > listener_.samples_)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

}
