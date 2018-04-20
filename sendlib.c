#include "sendlib.h"
/* 
 * This function must NOT be modified.
 */
ssize_t lossy_sendto (float loss_ratio, unsigned int random_seed, int sockfd, const void * buf, size_t len, const struct sockaddr *dest_addr, socklen_t addrlen)
{
  int n;
  float f;
  static int init = 0;
  
  if (! init) {
    srand (random_seed);
    init = 1;
  }
  
  f = ((float) rand ()) / RAND_MAX;
  
  /* Simulate segment loss */
  if (f < loss_ratio) return len;
  
  return sendto(sockfd, buf, len, 0, dest_addr, addrlen);  
}
