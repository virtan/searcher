# Lexicon, 1 thread

set term aqua size 640 400 font 'Tahoma,16' enhanced
set title 'Lexicon random read test, 3000 words, 16 threads'
set logscale y
#set size 1,0.5
set yrange [100:1000000]
set xlabel 'test [№]'
set ylabel 'time [log(mcs)]'
set format y '%g'
set grid
set ytics 10
set label 'cpp' at 900, 500 left tc rgb 'dark-green'
set label 'java' at 900, 16000 left tc rgb 'dark-red'
plot 'lex_t16_3k_cpp.data' using 1:2 lt rgb 'dark-green' with dots notitle,\
     'lex_t16_3k_java.data' using 1:2 lt rgb 'dark-red' with dots notitle
