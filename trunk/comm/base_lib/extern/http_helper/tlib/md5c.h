/*
	 MD5.H - header file for MD5C.C
	RSA Data Security, Inc. MD5 Message-Digest Algorithm
*/

#ifndef _MD5C_H
#define _MD5C_H

/* POINTER defines a generic pointer type */
typedef unsigned char * POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned int UINT4;

/* MD5 context. */
typedef struct {
  UINT4 state[4];                                           /* state (ABCD) */
  UINT4 count[2];                /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                                 /* input buffer */
} MD5_CTX;

void MD5cInit(MD5_CTX *);
void MD5cUpdate(MD5_CTX *, unsigned char *, unsigned int);
void MD5cFinal(unsigned char [16], MD5_CTX *);

#endif


