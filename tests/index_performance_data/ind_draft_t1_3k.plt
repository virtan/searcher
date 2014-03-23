# Lexicon, 1 thread

set term aqua size 700 500 font 'Tahoma,16' enhanced
set title 'Index random term posting scan test, 3k words, 1 thread, draft, 1 and 3 blocks'
#set logscale y
#set size 1,0.5
set yrange [0:100]
set xlabel 'test [№]'
set ylabel 'time [ms]'
set format y '%g'
set grid
set ytic 10
set label 'cpp 1bl' at 850, 10 left tc rgb 'dark-green'
set label 'java 1bl' at 850, 55 left tc rgb 'dark-red'
set label 'cpp 3bl' at 850, 35 left tc rgb 'green'
set label 'java 3bl' at 850, 90 left tc rgb 'red'
plot 'ind_1_1_t1_3k_cpp.data' using 1:($2/1000) lt rgb 'dark-green' with dots notitle,\
     'ind_1_1_t1_3k_java.data' using 1:($2/1000) lt rgb 'dark-red' with dots notitle,\
     'ind_3_1_t1_3k_cpp.data' using 1:($2/1000) lt rgb 'green' with dots notitle,\
     'ind_3_1_t1_3k_java.data' using 1:($2/1000) lt rgb 'red' with dots notitle
