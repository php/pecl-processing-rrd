threre are some files for testing

- speed.rrd - rrd database from rrd web site tutoril - http://www.mrtg.org/rrdtool/tut/rrdtutorial.en.html
 It was created via command
 sh$ rrdtool create speed.rrd --start 920804400 \
  DS:speed:COUNTER:600:U:U \
  RRA:AVERAGE:0.5:1:24 \
  RRA:AVERAGE:0.5:6:10 \

 and updated with data
 sh$ rrdtool update speed.rrd 920804700:12345 920805000:12357 920805300:12363
 sh$ rrdtool update speed.rrd 920805600:12363 920805900:12363 920806200:12373
 sh$ rrdtool update speed.rrd 920806500:12383 920806800:12393 920807100:12399
 sh$ rrdtool update speed.rrd 920807400:12405 920807700:12411 920808000:12415
 sh$ rrdtool update speed.rrd 920808300:12420 920808600:12422 920808900:12423

- speed.png - corresponding graph from speed.rrd from tutorial, command
 sh$ rrdtool graph speed.png \
  --start 920804400 --end 920808000 \
  --vertical-label m/s \
  DEF:myspeed=$rrdFile:speed:AVERAGE \
  CDEF:realspeed=myspeed,1000,* \
  LINE2:realspeed#FF0000
