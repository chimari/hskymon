gcc -o example example.c `pkg-config --cflags --cflags glib-2.0` `xml2-config --cflags` -lvotable -lxml2 `pkg-config --cflags --libs glib-2.0`
