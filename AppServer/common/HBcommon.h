#ifndef HB_ERROR_H_
#define HB_ERROR_H_

#include "common.h" // "common.h" contains general common shortcuts

//#define MB (1024*1024)
#define MB (1000*1000)
#define kMessageQueueSizeLimit (2000)
#define kFileNumberLimit (5)
//cushion size
#define kMsgQueuesCushionLimit (10)
#define kOffsetTableSizeLimit (8*MB)

#define kSingleFileSizeLimit (100*MB)
#define kSingleMsgSizeLimit (1*MB)
#define kLocalBufSizeLimit (1*MB)
#define kGlobalBufSizeLimit (200*MB)

//file path and filename prefix
#define kStrLogPath "./"
#define kStrFilenamePrefix "my_logfile_"
#define kFilenameLength 128

//define test message from producer
//define single message length
#define kMsgLen (1000-sizeof(frame_t))
//total number of messages to send
//#define kMsgNum (1000*1000*1)
#define kMsgNum (10)

//define ack message for producer
#define kAckLength sizeof(kAckStr)
#define kAckStr "SGack"

#define kStrServerPort "22223"
#define kServerPort 22223
#define kStrTailorPort "22221"
#define kTailorPort 22221

#define kServerTimeout 600
#define kTailorTimeout 600

//ack messages for consumer
#define kStrCurrentPos "currentPos"
//ack for currentPos is filenameNumber, offset, size, msgID
#define kAckCurrentPosLen (2*sizeof(int) + 2*sizeof(uint64_t))
#define kStrRewind "rewind"
#define kStrAckRewindOK "Rewind_OK"
#define kAckRewindOKLen (sizeof(kStrAckRewindOK))
#define kStrAckRewindBadPos "Rewind_BadPos"
#define kStrNextEvent "nextEvent"
enum errorCode {
  OK = 0,
  BAD_BUFFER,
  BAD_XML,
  MALFORMED_COMMENT,
  OUT_OF_MEMORY,
  HB_TRY_AGAIN,
  BUFFER_TOO_SMALL,
  MYSQL_IS_CRAZY,
  SERVER_IS_CRAZY,
  NOT_READY,
  INVALID_SG_POS,
  BAD_MSG,
  BAD_ADDR,
  SEND_FAIL,
  RECV_FAIL,
  UNKNOWN_EXCEPTION
};

typedef enum errorCode ErrorCode;

typedef uint32_t frame_t;
typedef uint64_t serialnum_t;

#endif
