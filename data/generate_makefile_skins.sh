#!/bin/sh
echo SKIN_SOURCES:=\\
while read skinname
do
	skinpath=../../skins/_old/"$skinname"
	for f in $skinpath/*.{asm,ASM}
	do
		if [ ! -f "$f" ]; then continue; fi
		if expr "$f" : '.*\.dtp' > /dev/null; then continue; fi
		echo -n allskins/$skinname.skn | sed 's/ /|/g'
		echo -n ':Skins/:'
		echo -n $f | sed 's/ /|/g'
		echo " \\"
	done
	for f in ../../skins/_old/"$skinname"/*/*.{asm,ASM}
	do
		if [ ! -f "$f" ]; then continue; fi
		if expr "$f" : '.*\.dtp' > /dev/null; then continue; fi
		g=`basename "$(dirname "$f")"`
		echo -n allskins/$g.skn | sed 's/ /|/g'
		echo -n ':Skins/:'
		echo -n $f | sed 's/ /|/g'
		echo " \\"
	done
done
echo '# end of list'
