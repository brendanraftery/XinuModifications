/* pipe.h: contains all the macros and definition for the pipe type in XINU */

#define MAXPIPES 10
#define PIPE_SIZE 1024

enum pipe_state_t {
	PIPE_FREE,
	PIPE_USED,
	PIPE_CONNECTED,
	PIPE_OTHER,
  PIPE_RDC,
  PIPE_WDC
};

struct pipe_t {
  char buffer[PIPE_SIZE];
 	pipid32 pipid;			    // Pipe ID
	enum pipe_state_t state;	// Pipe state defined by the enum
  pid32 owner;
  did32 device;
  int16 old_read;
  int16 old_write;
  int32 start;
  int32 end;
  int32 cnt;
  pid32 read_proc;
  pid32 write_proc;
  sid32 count_sem;
  sid32 space_sem;
};

// Possibly init syscalls

// Init sems
sid32 create_sem;
sid32 con_sem;
sid32 dc_sem;
sid32 del_sem;
sid32 get_sem;
sid32 put_sem;
struct pipe_t pipe_tables[MAXPIPES];	// Table for all pipes




