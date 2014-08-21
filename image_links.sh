#!/bin/bash
  mkdir -p imgs imgs_small pages
  
  if false
  then
    rm -f `find ./imgs_small`
    for p in `cat pages_list`
    do
      for i in `seq 10`
      do
        wget -U mozilla -w 0.5 -e robots=off -P pages $p/$i.html
      done      
    done
  fi
  
  if false
  then
    cat pag es/* | grep jpg | grep rightinthebox.com/images | sed 's/.*src="//' | sed 's/".*//' | grep -v ? >.tmp
    rm imgs/*
    wget  -U mozilla -w 0.5 -e robots=off -P imgs -i .tmp
  fi

  rm -f `find ./imgs_small`
  for f in `ls -1 imgs`
  do
    convert imgs/$f -resize 30x30 `echo imgs_small/$f | sed 's/.jpg/.bmp/'`
  done
