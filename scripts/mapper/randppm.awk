#!/bin/awk -f

#	Create random colors for colormap files

BEGIN {
  printf("P3\n16 16\n255\n");
  srand();
  printf("0 192 255\n");
  for (i=1;i<253;i++) {
    printf("%d %d %d\n",32*int(rand()*6)+32,32*int(rand()*6)+32,32*int(rand()*6)+32);
  }
  printf("255 255 255\n");
  printf("128 128 128\n");
  printf("0 0 0\n");
}
