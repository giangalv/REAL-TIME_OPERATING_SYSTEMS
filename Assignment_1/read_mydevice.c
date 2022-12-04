 /* 
 read_mydevice.c
 */
 
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
 
int main() 
{
  int fd, result;
  int len = 11;
  char buf[len];

  //The first call uses the device /dev/mydevice with minor number 0, the 
  //second call uses the device /dev/mydevice with minor number 1
  //the read and the write (first_RTOS) must use the same device to exchange 
  //data, since two different devices correspond to different areas of
  //memory

  if ((fd = open("/dev/mydevice", O_RDWR )) == -1) {
    perror("open");
    return 1;
  }

  if ((result = read(fd, &buf, sizeof(buf))) == len) {
    perror("read");
    return 1;
  }

  fprintf (stdout, "read %d bytes: %s from /dev/mydevice with minor number 0\n", result, buf);
  close(fd);
  return 0;
}