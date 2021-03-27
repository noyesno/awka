# SCGI Request
# ------------
#    $size:$name\0$value\0$name\0$value\0,$body

BEGIN {
  RS = ":";
  getline size;
  size = size+0;

  RS = "\0";
  while(size>0){
    getline name;
    getline value;
    print name "=" value;

    if(NR==3){
      if( name !="CONTENT_LENGTH" ){
        print "Error: The first header MUST BE 'CONTENT_LENGTH'";
      }
    } 
    req_header[name] = value;

    size = size - length(name) - length(value) - 2;
  }

  if( !("SCGI" in req_header && req_header["SCGI"] == "1") ) {

  }

  body_size = req_header["CONTENT_LENGTH"] + 1;

  RS = body_size;
  getline body;
  print "------ body ------"
  print substr(body, 2)
  print "------------------"

  RS = "\n"
}

{
  print; 
}
