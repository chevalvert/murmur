#! /bin/zsh

DIR="public/videos"
echo "Concatenating $DIR/transition-‰d3.mp4"

for file in $DIR/transition-*.mp4; do
  echo file "$file" >> .concat.txt
done

ffmpeg -safe 0 -f concat -i .concat.txt -c copy $DIR/transitions.mp4
rm -f .concat.txt
