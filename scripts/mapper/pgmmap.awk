#!/bin/awk -f

#
#	Convert a deity dump file into a Portable GreyMap of country numbers
#

function abs(x) {
  if (x>0) return x;
  return -x;
}

/own +sect/ {
  getline;
  while (NF>2) {
    own[$2]=$1;
    des[$2]=substr($3,0,1);
    if ($1>max) max=$1;
    getline;
  }
}

/own x y des/ {
  getline;
  while (NF>2) {
    own[$2,$3]=$1;
    des[$2,$3]=$4;
    if ($1>max) max=$1;
    if ($2 < left) left=$2;
    if ($2 > right) right=$2;
    if ($3 < top) top=$3;
    if ($3 > bottom) bottom=$3;
    getline;
  }
}

/^World size is [0-9]+ by [0-9]+./ {
  width = $4;
  height= $6 + 0;
  left=-width/2;
  right=width/2-1;
  top= -height/2;
  bottom = height/2-1;
}

END {
  printf("P2\n%d %d\n255\n",right-left+2,bottom-top+1);
  for (y=top;y<=bottom;y++) {
    printf("\n");
    if (y%2) {
      color=own[right,y];
      if (color==0 && des[right,y]!=".") color=254;
      printf("%d\n",color);
    }
    for (x=left + abs(y%2);x<=right;x+=2) {
      color=own[x,y];
      if (color==0 && des[x,y]!=".") color=254;
      printf("%d %d\n",color,color);
    }
    if (!(y%2)) {
      color=own[left,y];
      if (color==0 && des[left,y]!=".") color=254;
      printf("%d\n",color);
    }
  }
}

