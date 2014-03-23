# Lexicon, 240 threads

set term aqua size 640 300 font 'Tahoma,16' enhanced
set title 'Lexicon random read test, 2740 words, 240 threads'
set logscale y
#set size 1,0.5
set yrange [1:500000]
set xlabel 'test [№]'
set ylabel 'time [log(mcs)]'
set format y '%g'
set label 'cpp' at 900, 100 left tc rgb 'dark-green'
set label 'java' at 900, 10000 left tc rgb 'dark-red'
plot 'lex_t240_cpp.data' using 1:($2/1000) lt rgb 'dark-green' with lines notitle,\
     'lex_t1_java.data' using 1:($2/1000) lt rgb 'dark-red' with lines notitle
