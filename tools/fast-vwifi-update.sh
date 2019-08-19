#!/bin/bash

# Script : fast-vwifi-update
# Ecris par : David Ansart
# Ecris pour : Live Raizo / http://live-raizo.sourceforge.net
# But : Mettre à jour vwifi-server avec les positions des VMs dans GNS3

LOGIN=user
PASSWORD=user
SERVER=localhost

CERROR='\e[0;1;31m' # Rouge
SansCouleur='\e[0;m'

OptionVWifiQEmu=vwifi0

if [ -z "$(pgrep -xf '/usr/bin/python3 /usr/bin/gns3.*')" ]
then
	>&2 echo -e "${CERROR}Error: Your GNS3 project must be open !!!${SansCouleur}"
	exit 2
fi

if [ -z "$(pgrep -xf './vwifi-server')" ]
then
	>&2 echo -e "${CERROR}Error: vwifi-server is not started !!!${SansCouleur}"
	exit 2
fi

# récupère l'identifiant du projet
IdProjet=$(wget -q --user="$LOGIN" --password="$PASSWORD" "http://${SERVER}:3080/v2/projects" -O - | jq -M -r --unbuffered '.[] | select (.status=="opened") | .project_id')
if [ -z "${IdProjet}" ]
then
	>&2 echo -e "${CERROR}Error: No project is open !!!${SansCouleur}"
	exit 2
fi

# récupère la configuration du projet
ConfigProjectGNS3="$(mktemp fast-vwifi-update.XXXXXXXXXX --tmpdir=/tmp)"
wget -q --user="$LOGIN" --password="$PASSWORD" "http://${SERVER}:3080/v2/projects/${IdProjet}/nodes" -O "${ConfigProjectGNS3}"

NbValue=0
VMWithWifi=false

IFS=$'\n'
for line in $(jq -M '.[] | select (.node_type=="qemu") | {x: .x, y: .y, z: .z , option: .command_line} ' "${ConfigProjectGNS3}")
do
	if [[ "${line}" =~ ^[[:space:]]+\"x\":.* ]]
	then
		((NbValue++))
		# attention au decallage du print (?)
		X=$(echo "${line}" | awk -F '[:, ]' '{print $5}')
	elif [[ "${line}" =~ ^[[:space:]]+\"y\":.* ]]
	then
		((NbValue++))
		# attention au decallage du print (?)
		Y=$(echo "${line}" | awk -F '[:, ]' '{print $5}')
	elif [[ "${line}" =~ ^[[:space:]]+\"z\":.* ]]
	then
		((NbValue++))
		# attention au decallage du print (?)
		Z=$(echo "${line}" | awk -F '[:, ]' '{print $5}')
	elif [[ "${line}" =~ ^[[:space:]]+\"option\":.* ]]
	then
		((NbValue++))
		CID=$(echo "${line}" | grep -E -o 'id='${OptionVWifiQEmu}',guest-cid=[[:digit:]]+' | sed 's/id='${OptionVWifiQEmu}',guest-cid=//')
		if [ -n "${CID}" ]
		then
			VMWithWifi=true
		fi
	fi

	if (( NbValue == 4 ))
	then
		NbValue=0
		if ${VMWithWifi}
		then
			./vwifi-ctrl set "${CID}" "${X}" "${Y}" "${Z}"
			VMWithWifi=false
		fi
	fi
done

rm -f "${ConfigProjectGNS3}"
