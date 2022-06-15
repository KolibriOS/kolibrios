/* This prints out all the ASCII characters in the format pbm2df expects them
 * You can run this on an xterm with an 8x13 font, and put the resulting
 * screenshot straight into pbm2df!
 */

main()
{
  unsigned char i;
  for(i = ' '; i < 128; ++i)
  {
    printf("%c", i);
    if((i % 32) == 31) printf("\n");
  }
  return 0;
}
