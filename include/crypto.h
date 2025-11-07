/*Import function from OpenSSL but still stay compatible with C90*/

#define SHA1_DIGEST_LENGTH 20
unsigned char *SHA1(const unsigned char *d, unsigned long n, unsigned char *md);
