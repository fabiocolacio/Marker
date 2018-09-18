#!/bin/bash

echo "Generating .pot file..."
xgettext --from-code=UTF-8 --files-from=po/POTFILES --output=po/marker.pot

if [ $# = 0 ]; then
	echo "No parameter, exiting now."
	exit 1
fi

IFS='
'

if [ $1 = "--all" ]; then
	liste=`ls ./po/*po`

	for fichier in $liste
	do
		echo "Updating translation for: $fichier"
		msgmerge $fichier ./po/marker.pot > $fichier.temp.po
		mv $fichier.temp.po $fichier
	done

else
	for fichier in $@
	do
		echo "Updating translation for: $fichier"
		msgmerge ./po/$fichier.po ./po/marker.pot > ./po/$fichier.temp.po
		mv ./po/$fichier.temp.po ./po/$fichier.po
	done
fi

exit 0