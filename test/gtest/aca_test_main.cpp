// Copyright 2019 The Alcor Authors.
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

#include "aca_log.h"
#include "aca_util.h"
#include "gtest/gtest.h"
#include "goalstate.pb.h"
#include "aca_message_pulsar_producer.h"
#include <unistd.h> /* for getopt */

using namespace std;
using namespace aca_message_pulsar;

#define ACALOGNAME "AlcorControlAgentTest"

// Global variables
static char EMPTY_STRING[] = "";
string g_ofctl_command = EMPTY_STRING;
string g_ofctl_target = EMPTY_STRING;
string g_ofctl_options = EMPTY_STRING;

// total time for execute_system_command in microseconds
std::atomic_ulong g_initialize_execute_system_time(0);
// total time for execute_ovsdb_command in microseconds
std::atomic_ulong g_initialize_execute_ovsdb_time(0);
// total time for execute_openflow_command in microseconds
std::atomic_ulong g_initialize_execute_openflow_time(0);
// total time for execute_system_command in microseconds
std::atomic_ulong g_total_execute_system_time(0);
// total time for execute_ovsdb_command in microseconds
std::atomic_ulong g_total_execute_ovsdb_time(0);
// total time for execute_openflow_command in microseconds
std::atomic_ulong g_total_execute_openflow_time(0);
// total time for vpcs_table_mutex in microseconds
std::atomic_ulong g_total_vpcs_table_mutex_time(0);
// total time for goal state update in microseconds
std::atomic_ulong g_total_update_GS_time(0);

bool g_debug_mode = true;
bool g_demo_mode = false;

string remote_ip_1 = "172.17.0.2"; // for docker network
string remote_ip_2 = "172.17.0.3"; // for docker network
uint neighbors_to_create = 10;

static string mq_broker_ip = "pulsar://localhost:6650"; //for the broker running in localhost
static string mq_test_topic = "my-topic";

//
// Test suite: pulsar_test_cases
//
// Testing the pulsar implementation where AlcorControlAgent is the consumer
// and aca_test is acting as producer
// Note: it will require a pulsar setup on localhost therefore this test is DISABLED by default
//   it can be executed by:
//
//     aca_tests --gtest_also_run_disabled_tests --gtest_filter=*DISABLED_pulsar_consumer_test
//
TEST(pulsar_test_cases, DISABLED_pulsar_consumer_test)
{
  int retcode = 0;
  const int MESSAGES_TO_SEND = 10;
  string message = "Test Message";

  ACA_Message_Pulsar_Producer producer(mq_broker_ip, mq_test_topic);

  for (int i = 0; i < MESSAGES_TO_SEND; i++) {
    retcode = producer.publish(message);
    EXPECT_EQ(retcode, EXIT_SUCCESS);
  }
}

static void aca_cleanup()
{
  ACA_LOG_DEBUG("%s", "==========INIT TIMES:==========\n");

  ACA_LOG_DEBUG("g_initialize_execute_system_time = %lu microseconds or %lu milliseconds\n",
                g_initialize_execute_system_time.load(),
                us_to_ms(g_initialize_execute_system_time.load()));

  ACA_LOG_DEBUG("g_initialize_execute_ovsdb_time = %lu microseconds or %lu milliseconds\n",
                g_initialize_execute_ovsdb_time.load(),
                us_to_ms(g_initialize_execute_ovsdb_time.load()));

  ACA_LOG_DEBUG("g_initialize_execute_openflow_time = %lu microseconds or %lu milliseconds\n",
                g_initialize_execute_openflow_time.load(),
                us_to_ms(g_initialize_execute_openflow_time.load()));

  ACA_LOG_DEBUG("%s", "==========EXECUTION TIMES:==========\n");

  ACA_LOG_DEBUG("g_total_execute_system_time = %lu microseconds or %lu milliseconds\n",
                g_total_execute_system_time.load(),
                us_to_ms(g_total_execute_system_time.load()));

  ACA_LOG_DEBUG("g_total_execute_ovsdb_time = %lu microseconds or %lu milliseconds\n",
                g_total_execute_ovsdb_time.load(),
                us_to_ms(g_total_execute_ovsdb_time.load()));

  ACA_LOG_DEBUG("g_total_execute_openflow_time = %lu microseconds or %lu milliseconds\n",
                g_total_execute_openflow_time.load(),
                us_to_ms(g_total_execute_openflow_time.load()));

  ACA_LOG_DEBUG("%s", "==========UPDATE GS TIMES:==========\n");

  ACA_LOG_DEBUG("g_total_update_GS_time = %lu microseconds or %lu milliseconds\n",
                g_total_update_GS_time.load(), us_to_ms(g_total_update_GS_time.load()));

  ACA_LOG_INFO("%s", "Program exiting, cleaning up...\n");

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  ACA_LOG_CLOSE();
}

int main(int argc, char **argv)
{
  int option;

  ACA_LOG_INIT(ACALOGNAME);

  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  testing::InitGoogleTest(&argc, argv);

  while ((option = getopt(argc, argv, "p:c:n:")) != -1) {
    switch (option) {
    case 'p':
      remote_ip_1 = optarg;
      break;
    case 'c':
      remote_ip_2 = optarg;
      break;
    case 'n':
      neighbors_to_create = std::stoi(optarg);
      break;
    default: /* the '?' case when the option is not recognized */
      fprintf(stderr,
              "Usage: %s\n"
              "\t\t[-m parent machine IP]\n"
              "\t\t[-c child machine IP]\n"
              "\t\t[-n neighbors to create (default: 10)]\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  int rc = RUN_ALL_TESTS();

  aca_cleanup();

  return rc;
}
