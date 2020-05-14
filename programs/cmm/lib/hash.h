/* hash function; Author PaulCodeman */


/*
String.prototype.hashCode = function() {
  var hash = 0, i, chr;
  if (this.length === 0) return hash;
  for (i = 0; i < this.length; i++) {
    chr   = this.charCodeAt(i);
    hash  = ((hash << 5) - hash) + chr;
    hash |= 0; // Convert to 32bit integer
  }
  return hash;
};
*/

inline dword hashCode(dword data, length)
{
	dword hash = 0;
	WHILE (length)
	{
		hash = hash << 5 - hash + DSBYTE[data];
		data++;
		length--;
	}
	RETURN hash;
}
