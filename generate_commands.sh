#!/bin/bash

master_host=localhost
master_port=1400
master_group_port=1230
reporter_range_start=1300
worker_range_start=1500
expected_worker_nodes=1

create_command () {
  echo "./genetic_actor --master-node-host=${master_host} --master-node-port=${master_port} --master-group-port=${master_group_port} --reporter-range-start=${reporter_range_start} --worker-range-start=${worker_range_start} --expected-worker-nodes=${expected_worker_nodes} --mode=$1"
}

alias genetic_master="$(create_command MASTER)"
alias genetic_worker="$(create_command WORKER)"
alias genetic_reporter="$(create_command REPORTER)"
