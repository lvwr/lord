#include <iostream>
#include <sstream>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <cinttypes>
#include <cstring>
using namespace std;

#define OPC_VALID (opcode == 0 || opcode == 1 || opcode == 2 || opcode == 3 || opcode == 4 || opcode == 5 || opcode == 6)
#define MAX_OPCODE 7

/// Defs
typedef unsigned long int uInt64;

/// Prototypes
bool handleMessage();
bool handleCallImm();
bool handleCallEv();
bool handleRetImm();
bool handleRet();

uInt64 opcode;

/// Global vars
//map<uInt64, bool> monitorTable;
//map<uInt64, bool> monitorTable;
map<uInt64, int> monitorTable;
uInt64 histogram[MAX_OPCODE];

typedef struct{
  uInt64 op;
  uInt64 addr;
  uInt64 target;
  uInt64 exp_ret;
} trace_entry;

typedef struct{
  uInt64 op;
  uInt64 addr;
  uInt64 target;
} ret_entry;

ret_entry ret_buf;
trace_entry call_buf;
int warnings = 0;

/// Main entry point
int main(int argc, char *argv[]) {
  int received;

  printf("Lord initializing...\n");

  while ( true ) {

    received = fread(&opcode, sizeof(opcode), 1, stdin);

    if (received == 0) {
      break;
    }
    switch (opcode) {
      /// Message
    case 0:
      //printf("message\n");
      histogram[0]++;
      handleMessage();
      break;

      /// Call immediate
    case 1:
      //printf("opcode 1\n");
      histogram[1]++;
      handleCallImm();
      break;

    case 5:
      //printf("opcode 5\n");
      histogram[5]++;
      handleCallImm();
      break;

      /// Call Ev
    case 2:
      //printf("opcode 2\n");
      histogram[2]++;
      handleCallEv();
      break;

    case 6:
      //printf("opcode 6\n");
      histogram[6]++;
      handleCallEv();
      break;

      /// Ret immediate
    case 3:
      //printf("ret3\n");
      histogram[3]++;
      handleRetImm();
      break;

      /// Ret
    case 4:
      //printf("ret4\n");
      histogram[4]++;
      handleRet();
      break;

    default:
      printf("Warning: Unknown opcode %lx\n", opcode);
      break;
    }
  }

  printf("HISTOGRAM\n-\n");

  for (int i=0; i<MAX_OPCODE; i++) {
    printf("%d, %lu\n", i, histogram[i]);
  }
  printf("Warnings: %d\n", warnings);

  return 0;
}


bool handleMessage() {
  char message[100];

  /// Read the ','
  fgets(message, 100, stdin);

  if (message[strlen(message)-1] != '\n') {
    printf("didn't read the entire message\n");
    //scanf("%*[^\n]\n");
  }
  else {
    message[strlen(message)-1] = '\0';
  }

  printf("Lord: %s\n", message);

  return true;
}

bool handleCallImm() {
  fread(&call_buf.addr,sizeof(uInt64),3,stdin);
  //printf("call: %lx\n", call_buf.exp_ret);
  if(monitorTable.find(call_buf.exp_ret) == monitorTable.end()){
    monitorTable[call_buf.exp_ret] = 1;
  } else {
    monitorTable[call_buf.exp_ret] = monitorTable[call_buf.exp_ret] + 1;
  }

  return true;
}

bool handleCallEv() {
  fread(&call_buf.addr,sizeof(uInt64),3,stdin);
  //printf("call: %lx\n", call_buf.exp_ret);
  if(monitorTable.find(call_buf.exp_ret) == monitorTable.end()){
    monitorTable[call_buf.exp_ret] = 1;
  } else {
    monitorTable[call_buf.exp_ret] = monitorTable[call_buf.exp_ret] + 1;
  }

  return true;
}

bool handleRetImm() {
  fread(&ret_buf.addr,sizeof(uInt64),2,stdin);
  //printf("ret: %lx\n", ret_buf.target);

  if(monitorTable.empty()){
    printf("Warning: Return Monitor Table is empty. RET at 0x%lx returning\
           to address 0x%lx\n", ret_buf.addr, ret_buf.target);
    warnings++;
    return false;
  }

  map<uInt64, int>::iterator targetEntry = monitorTable.find(ret_buf.target);

  if(targetEntry == monitorTable.end()){
    printf("Warning: RET at 0x%lx returned to unexpected address at 0x%lx\n",
           ret_buf.addr, ret_buf.target);
    warnings++;
    return false;
  } else {
    if(monitorTable[ret_buf.target] < 0)	monitorTable.erase(targetEntry);
    else monitorTable[ret_buf.target] = monitorTable[ret_buf.target] - 1;
    return true;
  }
}

bool handleRet() {
  fread(&ret_buf.addr,sizeof(uInt64),2,stdin);
  //printf("ret: %lx\n", ret_buf.target);

  if(monitorTable.empty()){
    printf("Warning: Return Monitor Table is empty. RET at 0x%lx returning to\
           address 0x%lx\n", ret_buf.addr, ret_buf.target);
    warnings++;
    return false;
  }

  map<uInt64, int>::iterator targetEntry = monitorTable.find(ret_buf.target);

  if(targetEntry == monitorTable.end()){
    printf("Warning: RET at 0x%lx returned to unexpected address 0x%lx\n",
           ret_buf.addr, ret_buf.target);
    warnings++;
    return false;
  } else {
    if(monitorTable[ret_buf.target] < 0) monitorTable.erase(targetEntry);
    else monitorTable[ret_buf.target] = monitorTable[ret_buf.target] - 1;
    return true;
  }
}
