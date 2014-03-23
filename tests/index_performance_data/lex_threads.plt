# Lexicon, 1 thread

set term aqua size 640 300 font 'Tahoma,16' enhanced
set title 'Lexicon, reading 30k random terms 2400 times with different amount of threads'
#set logscale y
#set size 1,0.5
#set yrange [1:500000]
set xlabel 'threads'
set ylabel 'time [sec]'
#set format y '%g'
#set label 'cpp' at 900, 100 left tc rgb 'dark-green'
#set label 'java' at 900, 10000 left tc rgb 'dark-red'
2.08917647058824
plot 'different_threads_30k.data' using 1:($2*60+$3-2.089) lt rgb 'dark-red' with lines notitle
