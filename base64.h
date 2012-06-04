#ifndef _BASE64_H_
#define _BASE64_H_

// from http://qadpz.idi.ntnu.no/doxy/html/base64_8h-source.html

/*
  base64 is an encoding system where groups of three bytes are replaced by
  four characters. The character alphabet is picket to be compatible on ALL
  platforms. This is unlike the alphabet used by uuencode. See RFC1521

  The base64 API has four functions. The main functions are one for encoding,
  and one for decoding.
  The third one is for for calculating the exact needed buffer size
  when encoding. 
  The exact buffer size needed for decoding is impossible to
  predict without decoding. But it will NEVER be more than 3/4 of the
  size of the encoded data. The last function will however go through
  a set of data, stripping away anything not in the alphabet.

  NOTE: It is very important that the size of the tobuffers are large enough.
  The functions will not make sure the buffer van be used, it will blindly try
  to use it.

  The functions are
  
  b64encode() encodes binary data to the b64encode system. it has the option
  to format the output by adding newline characters at regular intervals
  given in number of four characters. A final newline will not be added
  after the last line, unless it naturally ends in a newline. That is
  when the exact number of characters written is the same as prescribed per
  line by the argument.

  b64decode() decodes base64 encoded data. It does not check for any
  heading identifying data, but assumes the buffer contains the data.
  It will stop before when the '=' end character is encountered, even if
  not the whole buffer is decoded. Will return -1 on error.

  Both b64encode() and b64decode() returns the actual number of bytes 
  written to the buffer.

  int b64strip_encoded_buffer() will walk through an encoded buffer
  and strip away all characters not in the alphabet. It will return
  the length of the now clean buffer.

  b64get_encode_buffer_size() calcuates and returns 
  the buffer size needed for encoding the given buffer. 

  The size must be at least four characters for every three in the 
  from buffer, and always an even number of four. For example one to three
  characters need four charcaters in the output buffer. Four to six
  characters need eight characters in the outputbuffer. And so forth.

  If output_quads_per_line is not zero, a newline character is added
  after every output_quads_per_line*4 characters. A final newline will
  not be written unless the line happens to be exact 
  output_quads_per_line*4 characters long.

  TODO: make sure decode handles all strange stuff

*/



  int b64encode (const char *from,char *to,int from_length, int output_quads_per_line = 0);
  int b64decode (const char *from,char *to,int from_length);
  int b64get_encode_buffer_size (int from_length,int output_quads_per_line = 0);
  int b64strip_encoded_buffer (char *buf,int length);


#endif
