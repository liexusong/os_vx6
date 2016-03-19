// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

static void consputc(int);

static int panicked = 0;

static struct {
  struct spinlock lock;
  int locking;
} cons;

static void
printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}
//PAGEBREAK: 50

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint*)(void*)(&fmt + 1);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if(locking)
    release(&cons.lock);
}

void
panic(char *s)
{
  int i;
  uint pcs[10];
  
  cli();
  cons.locking = 0;
  cprintf("cpu%d: panic: ", cpu->id);
  cprintf(s);
  cprintf("\n");
  getcallerpcs(&s, pcs);
  for(i=0; i<10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for(;;)
    ;
}

//PAGEBREAK: 50
#define BACKSPACE 0x100
#define MOVE_LEFT 0x101
#define MOVE_RIGHT 0x102
#define CRTPORT 0x3d4
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

static void
cgaputc(int c)
{
  int pos;
  
  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);

  if(c == '\n')
    pos += 80 - pos%80;
  else if(c == BACKSPACE || c == MOVE_LEFT) {
    if(pos > 0) --pos;
  }
  else if (c == MOVE_RIGHT) 
    pos++;
  else
    crt[pos++] = (c&0xff) | 0x0700;  // black on white

  if(pos < 0 || pos > 25*80)
    panic("pos under/overflow");
  
  if((pos/80) >= 24){  // Scroll up.
    memmove(crt, crt+80, sizeof(crt[0])*23*80);
    pos -= 80;
    memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
  }
  
  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);

  if (c != MOVE_LEFT && c != MOVE_RIGHT)
    crt[pos] = ' ' | 0x0700;    // put space in current position
}

void
consputc(int c)
{
  if(panicked){
    cli();
    for(;;)
      ;
  }

  if(c == BACKSPACE){
    uartputc('\b'); uartputc(' '); uartputc('\b');
  } else if (c == MOVE_LEFT || c == MOVE_RIGHT){
    while (0); //do nothing
  } else
    uartputc(c);  //write char to console
  cgaputc(c);     //change the cursor position
}

#define INPUT_BUF   128
#define MAX_HISTORY 16
struct {
  char buf[INPUT_BUF];
  uint r;  // Read index
  uint w;  // Write index - beginning of the current line?
  uint e;  // Edit index
  uint l;  // Write length - this is added because now we support caret navigation and edit index can change
} input;

#define C(x)  ((x)-'@')  // Control-x
static char historyBuffer[MAX_HISTORY][INPUT_BUF];
static int lastHistIdx = -1; 				// number of entered commands
static int histCmdIdx  = -1;				// distance (in steps) from user input - range [-1, 15]
static char userInput[INPUT_BUF]; 	// user input backup on arrows up/down

static void killline(void);

void 
cpycyclic(char *s, const char *t, int n){


}

//implemented as system call
int 
history(char* buffer, int historyId)
{
  if (historyId < 0 || 15 < historyId)  // history illegal
    return -2; 						
  else if (historyId > lastHistIdx)     // no history for the given id
    return -1; 						
  else {
  	safestrcpy(buffer, historyBuffer[(lastHistIdx-historyId) % MAX_HISTORY], INPUT_BUF);
    return 0;
  }
}

// Add new user command to the history array
void 
updateHistory(void)
{
  histCmdIdx = -1; // if we update history it means the user added a new cmd, so he is on "place" -1
  lastHistIdx++; // update the static index of commands in the history array
  char* src = historyBuffer[lastHistIdx % MAX_HISTORY];  
  memset(src, '\0', INPUT_BUF); // clear the slot in the history array and fill it with '\0'
  // copy from user input buffer to the history array:
  int i = 0;
  while (i < input.l){
    src[i] = input.buf[(input.w + i) % INPUT_BUF];
    i++;
  }
}

// Print to the console the commnds of history array using UP and DOWN keys
void 
showhist(uint histId){ // assume that histId has values -1 - 15
  char cmd[INPUT_BUF];
  if (histCmdIdx == -1) { // user input required
	safestrcpy(cmd, userInput, INPUT_BUF);
  }
  else {				   // another history cmd required
    history(cmd, histId);
  }

  //kill line - clean screen from old cmd
  killline();

  input.l = 0;
  input.e = input.w;
  while (cmd[input.l]) {
    input.buf[input.e++ % INPUT_BUF] = cmd[input.l];
    consputc(cmd[input.l]);
    input.l++;
  } 
  input.buf[input.e % INPUT_BUF] = '\0';
}

void 
backupUserInput(void){
  memset(userInput, '\0', INPUT_BUF);
  int i = 0;
    while (i < input.l){
	  userInput[i] = input.buf[(input.w + i) % INPUT_BUF];
	  i++;
	}
}

void 
moveup(void) 
{
  if (histCmdIdx < 15 && (histCmdIdx + 1) <= lastHistIdx) { 	// legal history cmd -- otherwise do nothing
  	histCmdIdx++;
  	if (histCmdIdx == 0 && input.e >= input.w) { // backup user input
      backupUserInput();
  	}
  	showhist(histCmdIdx);
  }
}

void 
movedown(void) 
{
  if (histCmdIdx > -1) {   //legal history cmd -- otherwise do nothing	
	histCmdIdx--; 
   	showhist(histCmdIdx);
  }
}

void
moveleft(void) 
{
  if(input.e != input.w){
    input.e--;
    consputc(MOVE_LEFT);
  }
}

void
moveright(void) 
{
  if (input.e != (input.w + input.l) % INPUT_BUF) {
    //consputc(input.buf[input.e % INPUT_BUF]);
    consputc(MOVE_RIGHT);
    input.e++;
  }
}

void
shiftoneleft(void) 
{
  char c;
  uint count = 0;
  while ((input.e % INPUT_BUF) < ((input.w + input.l) % INPUT_BUF)) 
  {
    c = input.buf[(input.e + 1) % INPUT_BUF];
    input.buf[input.e % INPUT_BUF] = c;
    input.e++;
    count++;
    consputc(c);
  }
  
  while (count > 0){
    moveleft();
    count--;
  } 
}

void
shiftoneright(int newc) 
{
  int tmp;
  int tmp2 = newc;
  uint count = 0;
  while ((input.e % INPUT_BUF) <= ((input.w + input.l) % INPUT_BUF)) 
  {
    consputc(tmp2);
    tmp = input.buf[(input.e) % INPUT_BUF]; 
    input.buf[input.e % INPUT_BUF] = tmp2;
    tmp2 = tmp;
    input.e++;
    count++;
  }

  while (count > 1){
    moveleft();
    count--;
  } 
}

void 
killline(void) {
  while (input.e != (input.w + input.l) % INPUT_BUF) { // move edit index to end
    moveright();
  }

  while(input.e != input.w &&
        input.buf[(input.e-1) % INPUT_BUF] != '\n'){
    input.e--;
    consputc(BACKSPACE);
  }

  input.l = 0; // init input length     
}

void
consoleintr(int (*getc)(void))
{
  int c, doprocdump = 0;

  acquire(&cons.lock);
  while((c = getc()) >= 0){
    switch(c){
    case C('P'):  // Process listing.
      doprocdump = 1;   // procdump() locks cons.lock indirectly; invoke later
      break;
    case C('U'):  // Kill line.
      killline();
      break;
    case C('H'): case '\x7f':  // Backspace
      if(input.e != input.w){
        input.e--;
        consputc(BACKSPACE);
        if(input.e + 1 != (input.w + input.l)){
          shiftoneleft();
        }
        input.l--;
      }
      break;
    case (0xE2): // Key up
      moveup();
      break;
    case (0xE3): // Key down
      movedown();
      break;
    case (0xE4): // Key left
      moveleft();
      break;
    case (0xE5): // Key right
      moveright();
      break;
    default:
      if(c != 0 && input.e-input.r < INPUT_BUF){
        c = (c == '\r') ? '\n' : c;              
        if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){ // user run a cmd (pressed ENTER key)           
          while (input.e < (input.w + input.l)){ // move the input buffer until the end 
            moveright();
          }
          if (input.w < input.e) // if the input isn't empty so update the history array
            updateHistory();
          input.buf[input.e++ % INPUT_BUF] = c;
          consputc(c);
         
          input.w = input.e;
          input.l = 0; //TODO: is it ok? maybe we should support multiline input?
          wakeup(&input.r);
        } 
        else if(input.e != (input.w + input.l)){
          //while(0);//do nothing
          input.l++;
          shiftoneright(c);
        } 
        else {
          input.l++;
          input.buf[input.e++ % INPUT_BUF] = c;
          consputc(c);
        }
      }
      break;
    }
  }
  release(&cons.lock);
  if(doprocdump) {
    procdump();  // now call procdump() wo. cons.lock held
  }
}

int
consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  iunlock(ip);
  target = n;
  acquire(&cons.lock);
  while(n > 0){
    while(input.r == input.w){
      if(proc->killed){
        release(&cons.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input.r, &cons.lock);
    }
    c = input.buf[input.r++ % INPUT_BUF];
    if(c == C('D')){  // EOF
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input.r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if(c == '\n')
      break;
  }
  release(&cons.lock);
  ilock(ip);

  return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
  int i;

  iunlock(ip);
  acquire(&cons.lock);
  for(i = 0; i < n; i++)
    consputc(buf[i] & 0xff);
  release(&cons.lock);
  ilock(ip);

  return n;
}

void
consoleinit(void)
{
  initlock(&cons.lock, "console");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  cons.locking = 1;

  picenable(IRQ_KBD);
  ioapicenable(IRQ_KBD, 0);
}
