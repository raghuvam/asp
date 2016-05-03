#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

#define CDRV_IOC_MAGIC 'Z'
#define ASP_CHGACCDIR _IOW(CDRV_IOC_MAGIC, 1, int)

int main (int argc, char *argv[]) {
  int fd;
  char ch, write_buf[100], read_buf[10];
  int dir, rc, rr, rw, i, no_bytes;
  int offset, origin;
  
  fd = open(argv[1], O_RDWR);
	if(fd == -1) {
		printf("Error opening file %s", argv[1]);
		exit(-1);
	}
  
  printf("r = read from device \nw = write to device \nc = change direction \nl = lseek \ne = exit");
  
  while (1) {
    
    printf("\nenter command :");
    scanf("%c", &ch);
    switch (ch) {
      case 'w':
			  printf("Enter new data to write: ");
			  scanf("%s", write_buf);
			  rw = write(fd, write_buf, strlen(write_buf));
        printf ("%d bytes written\n", rw);
        break;
        
      case 'r':
        printf ("bytes to read: ");
        scanf ("%d", &no_bytes);
        rr = read(fd, read_buf, no_bytes);
        printf("device(%d bytes): ", rr);
        for (i=0; i<rr; i++) {
          printf ("%c", read_buf[i]);
        }
        printf ("\n");
        break;
        
      case 'c':
  		  printf("0 = regular \n1 = reverse \n");
  		  printf("enter direction: ");
  		  scanf("%d", &dir);
  		  rc = ioctl(fd, ASP_CHGACCDIR, &dir);
  		  if (rc == -1) {
          perror("\n***error in ioctl***\n");
  		    return -1; 
        }
        printf ("\nold direction was %d\n", dir);
        break;
        
      case 'l':
        printf("Origin \n0 = beginning \n1 = current \n2 = end \n");
    		printf("enter origin: ");
    		scanf("%d", &origin);
    		printf("enter offset: ");
    		scanf("%d", &offset);
    		lseek(fd, offset, origin);
        break;
      case 'e':
        close(fd);
        exit (0);
        break;
        
      default:
        printf("Command not recognized\n");
        break;
    }
    while ( getchar() != '\n' );
  }
  return 0;
}