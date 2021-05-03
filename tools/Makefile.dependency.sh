#!/bin/bash

SRC="src"

EXT_H='h'
EXT_C='cc'
EXT_O='o'

findAllH()
{
	local file
	for file in $@
	do
		local includeH=$( grep -o '^[[:space:]]*#include[[:space:]]*\".*\"' ${SRC}/${file} | cut -d '"' -f 2 )
		if [ -n "${includeH}" ]
		then
			# not empty
			echo ${includeH} | xargs -n 1
			${FUNCNAME} ${includeH}
		fi
	done
}

findAllDependancies()
{
	local file
	for file in $@
	do
		local fileWithoutExt="${file%*.*}"
		local extension
		for extension in ${EXT_H} ${EXT_C}
		do
			if [ -e "${SRC}/${fileWithoutExt}.${extension}" ]
			then
				local includeH=$( grep -o '^[[:space:]]*#include[[:space:]]*\".*\"' ${SRC}/${fileWithoutExt}.${extension} | grep -v "\"${fileWithoutExt}.${EXT_H}\"" | cut -d '"' -f 2 )
				if [ -n "${includeH}" ]
				then
					# not empty
					echo ${includeH} | xargs -n 1
					${FUNCNAME} ${includeH}
				fi
			fi
		done
	done
}

transform_h_to_o()
{
	for fileH in $(cat)
	do
		fileWithoutDotH=${fileH%*.${EXT_H}}
		if [ -e "${SRC}/${fileWithoutDotH}.${EXT_C}" ]
		then
			echo '$(OBJ)/'"${fileWithoutDotH}.${EXT_O}"
		fi
	done
}

formatDisplay()
{
	sort -u | xargs
}

do_Bin_in_Makefile()
{
	for target in $@
	do
		echo "${target%*.${EXT_C}} : \$(OBJ)/${target%*.${EXT_C}}.${EXT_O}  $( findAllDependancies ${target} | transform_h_to_o | formatDisplay )"
		echo
	done
}

do_Obj_in_Makefile()
{
	for target in $@
	do
		echo '$(OBJ)/'"${target%*.${EXT_C}}.${EXT_O} : ${target} $( findAllH ${target} | formatDisplay ) Makefile Makefile.in"
		echo
	done
}

do_Obj_in_Makefile $( find ${SRC} -type f -name "*.${EXT_C}" -printf "%f\n" )  > Makefile.in

do_Bin_in_Makefile $( grep -E -l '^[[:space:]]*int[[:space:]]+main[[:space:]]*\(' ${SRC}/*.${EXT_C} | xargs -L 1 basename ) >> Makefile.in
