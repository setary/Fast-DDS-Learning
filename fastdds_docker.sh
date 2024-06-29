#! /bin/bash

if [[ $1 == 0 ]];then
  docker load < ./fastdds_learning.tar
elif [[ $1 == 1 ]] && [[ $# == 3 ]]; then
  work_dir=$2
  docker_dir=/FastDDS
  container=$3
  docker run -it -d -v $work_dir:$docker_dir --name ${container} fastdds_learning:24.04.20.01 /bin/bash
  docker exec -it ${container} /bin/bash
elif [[ $1 == 2 ]]; then
  container=$2
  docker exec -it ${container} /bin/bash
else
  echo 'parameter error'
fi

