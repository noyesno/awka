#!/usr/local/bin/perl

# Not an RE, but compares using index to find a substring
# against re2.awk which uses an RE
#
$base = 100000;

$s = "3c21e03a10b9415fb3e1067ea75f8205\
c8dc9900a5089d31e01241c7a947ed7e\
d5f8cd6bb86ebef6d7d104c84ae6e8a7\
e23c99af9c9d6d0294d8b51094c39021\
4bb4af7e61760735ba17c29e8f542a66\
875da91e90863f1ddb7e149297fc59af\
cf5de951fb65d06d2927aab7b9b54830\
e2d935616a54c381c2f38db3731d5a37\
SGVsbG8gbXk\
6dd11d15c419ac219901f14bdd999f38\
0ad94e978ad624d15189f5230e5435a9\
2dc19fe95e583e7d593dd52ae7e68a6e\
465ffa6074a371a8958dad3ad271181a\
23310939b981b4e56f2ecee26f82ec60\
fe04bef49be47603d1278cc80673b226\
gbmFtZSBpcy\
3c21e03a10b9415fb3e1067ea75f8205\
c8dc9900a5089d31e01241c7a947ed7e\
d5f8cd6bb86ebef6d7d104c84ae6e8a7\
e23c99af9c9d6d0294d8b51094c39021\
BvbGl2ZXIga\
4bb4af7e61760735ba17c29e8f542a66\
875da91e90863f1ddb7e149297fc59af\
cf5de951fb65d06d2927aab7b9b54830\
e2d935616a54c381c2f38db3731d5a37\
G9vcmF5IQ==";

$x = index($s, "G9vcmF5IQ==");
#print $x, "\n";

for($i=0; $i<$base; $i++) {
  $x = index($s, "G9vcmF5IQ==");
}
