#!/bin/awk -f

# Parse report and generate pbmtext commands for each country
# Now generates new positions as an average of all sectors.

BEGIN {
  if (scale=="") scale=1;
  FS="[ \t\n,]+";
}

/^World size is [0-9]+ by [0-9]+./ {
  width = $4;
  height= $6;
  system("ppmmake white " (width+1)*scale " " height*scale " | ppmtopgm | pgmtopbm >over.pbm");
}

/.*#.*name.*tech.*research.*education.*happiness.*cap-location/ {
  getline;
  while (NF>4) {
    if ($2>0 && $2<90) {
      name[$2]=substr($0,8,15);
      capx[$2]= $(NF-1);
      capy[$2]= $NF;
    }
    getline;
  }
}

/own +sect/ {
  getline;
  while (NF>2) {
    if ($2 in name) {
      if (!count[$2]) {
	cx[$2] = $3;
	cy[$2] = $4;
      } else {
	dx = $3 - cx[$2];
	dy = $4 - cy[$2];
	if (dx>=width/2) dx -= width;
	if (dy>=height/2) dy -= height;
	if (dx<-width/2) dx += width;
	if (dy<-height/2) dy += height;
	cx[$2] += dx/(1+count[$2]);
	cy[$2] += dy/(1+count[$2]);
      }
      count[$2]++;
    }
    getline;
  }
}


END {
  for (i in name) {
    if (count[i]) {
      system("pbmtext \"" name[i] "\" | pnmcrop >text.pbm");
      system("pnmfile text.pbm >text.size");
      getline < "text.size";
      close("text.size");
      h=$NF;
      w=$(NF-2);
      x = cx[i] + width/2;
      y = cy[i] + height/2;

      while (x<0) x += width;
      while (y<0) y += height;
      while (x>=width) x -= width;
      while (y>=height) y -= height;

      x = int(x*scale);
      y = int(y*scale);

      x -= w/2;
      y -= h/2;
      if (x<0) x=0;
      if (y<0) y=0;
      if (x+w>=width*scale) x=width*scale-w-1;
      if (y+h>=height*scale) y=height*scale-h-1;


      system("pnmpaste text.pbm " x " " y " over.pbm > over2.pbm");
      system("cp over2.pbm over.pbm");
    }
  }
}
