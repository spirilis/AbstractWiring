/^struct .* \{/ {
        allstructs[$2] = 1;
}

/^#define[ \t].*volatile struct.*0x/ {
        struct_name = $5;
        sfrname = $2;
        sfr_addr = substr($6,3, length($6)-3);
        if (allstructs[struct_name] == 1) {
                sfr[sfrname] = struct_name;
                sfraddr[sfrname] = sfr_addr;
        }
}

END {
        ldi = 1;

        ldscript_out = "rx_peripherals.ld";
        periph_header = "rx_peripherals.h";

        print "SECTIONS" > ldscript_out;
        print "{" >> ldscript_out;

        print "// RX peripheral constants derived from iodefine.h; these are suitable for const expressions in C++ templates." > periph_header;
        print "// Be sure to include the matching rx_peripherals.ld GNU GCC ldscript as a supplemental linker script in your build." >> periph_header;
        print "" >> periph_header;
        print "#ifndef RX_PERIPHERALS_H" >> periph_header;
        print "#define RX_PERIPHERALS_H" >> periph_header;
        print "" >> periph_header;
        print "#include <iodefine.h>" >> periph_header;
        print "" >> periph_header;

        for (sn in allstructs) {
                for (s in sfr) {
                        if (sfr[s] == sn) {
                                printf "Exporting information about %s (volatile struct %s)\n", s, sn;
                                printf "extern volatile struct %s SFRBASE_%s;\n", sn, s >> periph_header;
                                printf "  .sfr_%d (%s) :\n", ldi++, sfraddr[s] >> ldscript_out;
                                printf "  {\n" >> ldscript_out;
                                printf "    PROVIDE(_SFRBASE_%s = .);\n", s >> ldscript_out;
                                printf "  }\n" >> ldscript_out;
                                printf "\n" >> ldscript_out;
                        }
                }
        }

        print "" >> periph_header;
        print "#endif /* RX_PERIPHERALS_H */" >> periph_header;
        print "}" >> ldscript_out;
}
