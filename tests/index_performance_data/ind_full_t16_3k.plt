# Lexicon, 1 thread

set term aqua size 700 500 font 'Tahoma,16' enhanced
set title 'Index random term posting scan test, 3k words, 1 thread, full, 1 and 3 blocks'
#set logscale y
#set size 1,0.5
set yrange [0:300]
set xlabel 'test [№]'
set ylabel 'time [ms]'
set format y '%g'
set grid
set ytic 20
set label 'cpp 1bl' at 850, 25 left tc rgb 'dark-green'
set label 'java 1bl' at 850, 80 left tc rgb 'dark-red'
set label 'cpp 3bl' at 850, 130 left tc rgb 'green'
set label 'java 3bl' at 850, 170 left tc rgb 'blue'
plot 'ind_1_2_t16_3k_cpp.data' using 1:($2/1000) lt rgb 'dark-green' with dots notitle,\
     'ind_1_2_t16_3k_java.data' using 1:($2/1000) lt rgb 'dark-red' with dots notitle,\
     'ind_3_2_t16_3k_cpp.data' using 1:($2/1000) lt rgb 'green' with dots notitle,\
     'ind_3_2_t16_3k_java.data' using 1:($2/1000) lt rgb 'blue' with dots notitle
