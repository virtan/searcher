# Lexicon, 1 thread

set term aqua size 640 400 font 'Tahoma,16' enhanced
set title 'Lexicon memory consumption (for 46Mb source)'
set yrange [0:200]
set xrange [0:5]
set xlabel 'test'
set ylabel 'memory [Mb]'
set label 'source' at 0.8, 55 left tc rgb 'black'
set label 'java' at 1.9, 62 left tc rgb 'black'
set label 'java loading' at 2.6, 170 left tc rgb 'black'
set label 'c++' at 3.9, 110 left tc rgb 'black'
plot '-' using 1:2 lt rgb 'green' with boxes fs s notitle,\
    '' using 1:2 lt rgb 'blue' with boxes fs s notitle,\
    '' using 1:2 lt rgb 'red' with boxes fs s notitle,\
    '' using 1:2 lt rgb 'yellow' with boxes fs s notitle
0 0
1 46
2 0
e
2 52
3 0
e
3 160
4 0
e
4 102
5 0
e
