# Lexicon, 1 thread

set term aqua size 640 400 font 'Tahoma,16' enhanced
set title 'Lexicon random read test, 3000 words, c++'
set logscale y
#set size 1,0.5
set yrange [160:2800]
set xlabel 'test [№]'
set ylabel 'time [log(mcs)]'
set format y '%g'
set grid
set ytics 2
set label '1t' at 630, 300 left tc rgb 'green'
set label '16t' at 715, 300 left tc rgb 'magenta'
set label '240t' at 800, 300 left tc rgb 'blue'
set label '800t' at 900, 300 left tc rgb 'orange'
plot 'lex_t1_3k_cpp.data' using 1:2 lt rgb 'green' with dots notitle,\
     'lex_t16_3k_cpp.data' using 1:2 lt rgb 'magenta' with dots notitle,\
     'lex_t240_3k_cpp.data' using 1:2 lt rgb 'blue' with dots notitle,\
     'lex_t800_3k_cpp.data' using 1:2 lt rgb 'orange' with dots notitle
