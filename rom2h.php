<?php
	// This is utility (sorry written in PHP) for transforming .ROM file into C source code
	// Cannot include original ROMs because of licences
	// Usage: php rom2h.php <48.rom >rom_spectrum.h
	$rom = file_get_contents('php://stdin');
?>
#ifndef __ROM_H
#define __ROM_H

#include <avr/pgmspace.h>
const PROGMEM unsigned char ROM[] = {
<?php
	$len = strlen($rom);
	for ($i = 0; $i < $len; $i++) {
		echo("\t" . '0x' . str_pad(dechex(ord($rom{$i})), 2, '0', STR_PAD_LEFT));
		if ($i < $len - 1) {
			echo(',');
		}
		if (($i % 16) == 15) {
			echo("\n");
		}
	}
?>
};

#endif
