
farm.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <start_farm>:
   0:	55                   	push   %rbp
   1:	48 89 e5             	mov    %rsp,%rbp
   4:	b8 01 00 00 00       	mov    $0x1,%eax
   9:	5d                   	pop    %rbp
   a:	c3                   	retq   

000000000000000b <addval_227>:
   b:	55                   	push   %rbp
   c:	48 89 e5             	mov    %rsp,%rbp
   f:	89 7d fc             	mov    %edi,-0x4(%rbp)
  12:	8b 45 fc             	mov    -0x4(%rbp),%eax
  15:	2d a8 6b 6f 6f       	sub    $0x6f6f6ba8,%eax
  1a:	5d                   	pop    %rbp
  1b:	c3                   	retq   

000000000000001c <addval_135>:
  1c:	55                   	push   %rbp
  1d:	48 89 e5             	mov    %rsp,%rbp
  20:	89 7d fc             	mov    %edi,-0x4(%rbp)
  23:	8b 45 fc             	mov    -0x4(%rbp),%eax
  26:	2d 9d a7 6e 6f       	sub    $0x6f6ea79d,%eax
  2b:	5d                   	pop    %rbp
  2c:	c3                   	retq   

000000000000002d <addval_465>:
  2d:	55                   	push   %rbp
  2e:	48 89 e5             	mov    %rsp,%rbp
  31:	89 7d fc             	mov    %edi,-0x4(%rbp)
  34:	8b 45 fc             	mov    -0x4(%rbp),%eax
  37:	2d ec b7 76 38       	sub    $0x3876b7ec,%eax
  3c:	5d                   	pop    %rbp
  3d:	c3                   	retq   

000000000000003e <getval_352>:
  3e:	55                   	push   %rbp
  3f:	48 89 e5             	mov    %rsp,%rbp
  42:	b8 58 90 90 c3       	mov    $0xc3909058,%eax
  47:	5d                   	pop    %rbp
  48:	c3                   	retq   

0000000000000049 <setval_125>:
  49:	55                   	push   %rbp
  4a:	48 89 e5             	mov    %rsp,%rbp
  4d:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
  51:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
  55:	c7 00 45 aa 58 90    	movl   $0x9058aa45,(%rax)
  5b:	5d                   	pop    %rbp
  5c:	c3                   	retq   

000000000000005d <addval_317>:
  5d:	55                   	push   %rbp
  5e:	48 89 e5             	mov    %rsp,%rbp
  61:	89 7d fc             	mov    %edi,-0x4(%rbp)
  64:	8b 45 fc             	mov    -0x4(%rbp),%eax
  67:	2d b8 76 38 6b       	sub    $0x6b3876b8,%eax
  6c:	5d                   	pop    %rbp
  6d:	c3                   	retq   

000000000000006e <setval_239>:
  6e:	55                   	push   %rbp
  6f:	48 89 e5             	mov    %rsp,%rbp
  72:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
  76:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
  7a:	c7 00 48 89 c7 94    	movl   $0x94c78948,(%rax)
  80:	5d                   	pop    %rbp
  81:	c3                   	retq   

0000000000000082 <getval_139>:
  82:	55                   	push   %rbp
  83:	48 89 e5             	mov    %rsp,%rbp
  86:	b8 48 89 c7 c3       	mov    $0xc3c78948,%eax
  8b:	5d                   	pop    %rbp
  8c:	c3                   	retq   

000000000000008d <mid_farm>:
  8d:	55                   	push   %rbp
  8e:	48 89 e5             	mov    %rsp,%rbp
  91:	b8 01 00 00 00       	mov    $0x1,%eax
  96:	5d                   	pop    %rbp
  97:	c3                   	retq   

0000000000000098 <add_xy>:
  98:	55                   	push   %rbp
  99:	48 89 e5             	mov    %rsp,%rbp
  9c:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
  a0:	48 89 75 f0          	mov    %rsi,-0x10(%rbp)
  a4:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
  a8:	48 8b 55 f8          	mov    -0x8(%rbp),%rdx
  ac:	48 01 d0             	add    %rdx,%rax
  af:	5d                   	pop    %rbp
  b0:	c3                   	retq   

00000000000000b1 <setval_266>:
  b1:	55                   	push   %rbp
  b2:	48 89 e5             	mov    %rsp,%rbp
  b5:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
  b9:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
  bd:	c7 00 a9 c2 08 c0    	movl   $0xc008c2a9,(%rax)
  c3:	5d                   	pop    %rbp
  c4:	c3                   	retq   

00000000000000c5 <setval_472>:
  c5:	55                   	push   %rbp
  c6:	48 89 e5             	mov    %rsp,%rbp
  c9:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
  cd:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
  d1:	c7 00 89 ce 78 c0    	movl   $0xc078ce89,(%rax)
  d7:	5d                   	pop    %rbp
  d8:	c3                   	retq   

00000000000000d9 <addval_424>:
  d9:	55                   	push   %rbp
  da:	48 89 e5             	mov    %rsp,%rbp
  dd:	89 7d fc             	mov    %edi,-0x4(%rbp)
  e0:	8b 45 fc             	mov    -0x4(%rbp),%eax
  e3:	2d 77 3d c7 36       	sub    $0x36c73d77,%eax
  e8:	5d                   	pop    %rbp
  e9:	c3                   	retq   

00000000000000ea <addval_493>:
  ea:	55                   	push   %rbp
  eb:	48 89 e5             	mov    %rsp,%rbp
  ee:	89 7d fc             	mov    %edi,-0x4(%rbp)
  f1:	8b 45 fc             	mov    -0x4(%rbp),%eax
  f4:	2d 3e b5 76 1f       	sub    $0x1f76b53e,%eax
  f9:	5d                   	pop    %rbp
  fa:	c3                   	retq   

00000000000000fb <setval_460>:
  fb:	55                   	push   %rbp
  fc:	48 89 e5             	mov    %rsp,%rbp
  ff:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
 103:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
 107:	c7 00 f2 48 89 e0    	movl   $0xe08948f2,(%rax)
 10d:	5d                   	pop    %rbp
 10e:	c3                   	retq   

000000000000010f <setval_429>:
 10f:	55                   	push   %rbp
 110:	48 89 e5             	mov    %rsp,%rbp
 113:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
 117:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
 11b:	c7 00 73 b6 8d ce    	movl   $0xce8db673,(%rax)
 121:	5d                   	pop    %rbp
 122:	c3                   	retq   

0000000000000123 <addval_373>:
 123:	55                   	push   %rbp
 124:	48 89 e5             	mov    %rsp,%rbp
 127:	89 7d fc             	mov    %edi,-0x4(%rbp)
 12a:	8b 45 fc             	mov    -0x4(%rbp),%eax
 12d:	2d 57 2e 6f 6f       	sub    $0x6f6f2e57,%eax
 132:	5d                   	pop    %rbp
 133:	c3                   	retq   

0000000000000134 <setval_260>:
 134:	55                   	push   %rbp
 135:	48 89 e5             	mov    %rsp,%rbp
 138:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
 13c:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
 140:	c7 00 09 ce 84 c0    	movl   $0xc084ce09,(%rax)
 146:	5d                   	pop    %rbp
 147:	c3                   	retq   

0000000000000148 <setval_376>:
 148:	55                   	push   %rbp
 149:	48 89 e5             	mov    %rsp,%rbp
 14c:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
 150:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
 154:	c7 00 48 c9 e0 c3    	movl   $0xc3e0c948,(%rax)
 15a:	5d                   	pop    %rbp
 15b:	c3                   	retq   

000000000000015c <getval_297>:
 15c:	55                   	push   %rbp
 15d:	48 89 e5             	mov    %rsp,%rbp
 160:	b8 89 ce c7 69       	mov    $0x69c7ce89,%eax
 165:	5d                   	pop    %rbp
 166:	c3                   	retq   

0000000000000167 <setval_350>:
 167:	55                   	push   %rbp
 168:	48 89 e5             	mov    %rsp,%rbp
 16b:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
 16f:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
 173:	c7 00 89 d1 84 db    	movl   $0xdb84d189,(%rax)
 179:	5d                   	pop    %rbp
 17a:	c3                   	retq   

000000000000017b <getval_226>:
 17b:	55                   	push   %rbp
 17c:	48 89 e5             	mov    %rsp,%rbp
 17f:	b8 8b d1 20 d2       	mov    $0xd220d18b,%eax
 184:	5d                   	pop    %rbp
 185:	c3                   	retq   

0000000000000186 <setval_271>:
 186:	55                   	push   %rbp
 187:	48 89 e5             	mov    %rsp,%rbp
 18a:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
 18e:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
 192:	c7 00 48 89 e0 c3    	movl   $0xc3e08948,(%rax)
 198:	5d                   	pop    %rbp
 199:	c3                   	retq   

000000000000019a <getval_365>:
 19a:	55                   	push   %rbp
 19b:	48 89 e5             	mov    %rsp,%rbp
 19e:	b8 89 c2 18 d2       	mov    $0xd218c289,%eax
 1a3:	5d                   	pop    %rbp
 1a4:	c3                   	retq   

00000000000001a5 <setval_288>:
 1a5:	55                   	push   %rbp
 1a6:	48 89 e5             	mov    %rsp,%rbp
 1a9:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
 1ad:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
 1b1:	c7 00 48 89 e0 c7    	movl   $0xc7e08948,(%rax)
 1b7:	5d                   	pop    %rbp
 1b8:	c3                   	retq   

00000000000001b9 <getval_126>:
 1b9:	55                   	push   %rbp
 1ba:	48 89 e5             	mov    %rsp,%rbp
 1bd:	b8 b3 89 d1 c7       	mov    $0xc7d189b3,%eax
 1c2:	5d                   	pop    %rbp
 1c3:	c3                   	retq   

00000000000001c4 <getval_341>:
 1c4:	55                   	push   %rbp
 1c5:	48 89 e5             	mov    %rsp,%rbp
 1c8:	b8 09 ce 08 db       	mov    $0xdb08ce09,%eax
 1cd:	5d                   	pop    %rbp
 1ce:	c3                   	retq   

00000000000001cf <setval_300>:
 1cf:	55                   	push   %rbp
 1d0:	48 89 e5             	mov    %rsp,%rbp
 1d3:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
 1d7:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
 1db:	c7 00 89 c2 a4 d2    	movl   $0xd2a4c289,(%rax)
 1e1:	5d                   	pop    %rbp
 1e2:	c3                   	retq   

00000000000001e3 <addval_273>:
 1e3:	55                   	push   %rbp
 1e4:	48 89 e5             	mov    %rsp,%rbp
 1e7:	89 7d fc             	mov    %edi,-0x4(%rbp)
 1ea:	8b 45 fc             	mov    -0x4(%rbp),%eax
 1ed:	2d 77 2e 87 24       	sub    $0x24872e77,%eax
 1f2:	5d                   	pop    %rbp
 1f3:	c3                   	retq   

00000000000001f4 <setval_275>:
 1f4:	55                   	push   %rbp
 1f5:	48 89 e5             	mov    %rsp,%rbp
 1f8:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
 1fc:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
 200:	c7 00 89 c2 c1 c2    	movl   $0xc2c1c289,(%rax)
 206:	5d                   	pop    %rbp
 207:	c3                   	retq   

0000000000000208 <getval_106>:
 208:	55                   	push   %rbp
 209:	48 89 e5             	mov    %rsp,%rbp
 20c:	b8 30 89 d1 90       	mov    $0x90d18930,%eax
 211:	5d                   	pop    %rbp
 212:	c3                   	retq   

0000000000000213 <addval_137>:
 213:	55                   	push   %rbp
 214:	48 89 e5             	mov    %rsp,%rbp
 217:	89 7d fc             	mov    %edi,-0x4(%rbp)
 21a:	8b 45 fc             	mov    -0x4(%rbp),%eax
 21d:	2d b8 76 1f 3e       	sub    $0x3e1f76b8,%eax
 222:	5d                   	pop    %rbp
 223:	c3                   	retq   

0000000000000224 <addval_114>:
 224:	55                   	push   %rbp
 225:	48 89 e5             	mov    %rsp,%rbp
 228:	89 7d fc             	mov    %edi,-0x4(%rbp)
 22b:	8b 45 fc             	mov    -0x4(%rbp),%eax
 22e:	05 89 ce c3 6b       	add    $0x6bc3ce89,%eax
 233:	5d                   	pop    %rbp
 234:	c3                   	retq   

0000000000000235 <getval_312>:
 235:	55                   	push   %rbp
 236:	48 89 e5             	mov    %rsp,%rbp
 239:	b8 48 c9 e0 90       	mov    $0x90e0c948,%eax
 23e:	5d                   	pop    %rbp
 23f:	c3                   	retq   

0000000000000240 <setval_382>:
 240:	55                   	push   %rbp
 241:	48 89 e5             	mov    %rsp,%rbp
 244:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
 248:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
 24c:	c7 00 88 c2 c3 bf    	movl   $0xbfc3c288,(%rax)
 252:	5d                   	pop    %rbp
 253:	c3                   	retq   

0000000000000254 <addval_446>:
 254:	55                   	push   %rbp
 255:	48 89 e5             	mov    %rsp,%rbp
 258:	89 7d fc             	mov    %edi,-0x4(%rbp)
 25b:	8b 45 fc             	mov    -0x4(%rbp),%eax
 25e:	2d 77 3d 6f 6f       	sub    $0x6f6f3d77,%eax
 263:	5d                   	pop    %rbp
 264:	c3                   	retq   

0000000000000265 <setval_371>:
 265:	55                   	push   %rbp
 266:	48 89 e5             	mov    %rsp,%rbp
 269:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
 26d:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
 271:	c7 00 63 5b 81 d1    	movl   $0xd1815b63,(%rax)
 277:	5d                   	pop    %rbp
 278:	c3                   	retq   

0000000000000279 <getval_423>:
 279:	55                   	push   %rbp
 27a:	48 89 e5             	mov    %rsp,%rbp
 27d:	b8 48 89 e0 c2       	mov    $0xc2e08948,%eax
 282:	5d                   	pop    %rbp
 283:	c3                   	retq   

0000000000000284 <getval_202>:
 284:	55                   	push   %rbp
 285:	48 89 e5             	mov    %rsp,%rbp
 288:	b8 99 c2 84 d2       	mov    $0xd284c299,%eax
 28d:	5d                   	pop    %rbp
 28e:	c3                   	retq   

000000000000028f <setval_203>:
 28f:	55                   	push   %rbp
 290:	48 89 e5             	mov    %rsp,%rbp
 293:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
 297:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
 29b:	c7 00 89 ce 08 db    	movl   $0xdb08ce89,(%rax)
 2a1:	5d                   	pop    %rbp
 2a2:	c3                   	retq   

00000000000002a3 <setval_401>:
 2a3:	55                   	push   %rbp
 2a4:	48 89 e5             	mov    %rsp,%rbp
 2a7:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
 2ab:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
 2af:	c7 00 c9 d1 c3 c2    	movl   $0xc2c3d1c9,(%rax)
 2b5:	5d                   	pop    %rbp
 2b6:	c3                   	retq   

00000000000002b7 <getval_248>:
 2b7:	55                   	push   %rbp
 2b8:	48 89 e5             	mov    %rsp,%rbp
 2bb:	b8 89 ce 48 c0       	mov    $0xc048ce89,%eax
 2c0:	5d                   	pop    %rbp
 2c1:	c3                   	retq   

00000000000002c2 <end_farm>:
 2c2:	55                   	push   %rbp
 2c3:	48 89 e5             	mov    %rsp,%rbp
 2c6:	b8 01 00 00 00       	mov    $0x1,%eax
 2cb:	5d                   	pop    %rbp
 2cc:	c3                   	retq   
