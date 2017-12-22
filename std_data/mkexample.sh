gcc -o example example.c -lvotable `xml2-config --cflags --libs` `pkg-config glib-2.0 --cflags --libs` 
gcc -o mk_acker2000 mk_acker2000.c -lm -lvotable `xml2-config --cflags --libs` `pkg-config glib-2.0 --cflags --libs` 
