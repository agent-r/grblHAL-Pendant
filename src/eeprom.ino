
String EepromReadString(int Start, int Length) {
        String Result;
        for (int i = Start; i < (Start+Length); i++) {
                Result = Result + String(char(EEPROM.read(i)));
        }
        Result.trim();
        return(Result);
}

void EepromWriteString(String Write, int Start, int Length) {
        while (Write.length() < Length) {
                Write = Write + " ";
        }
        for (int i = 0; i < Length; i++) {
                EEPROM.write(i + Start, Write.charAt(i));
        }
        EEPROM.commit();
}

int EepromReadInt(int Start) {
        int Result = (EEPROM.read(Start) << 8) + EEPROM.read(Start+1);
        return(Result);
}

void EepromWriteInt(int Write, int Start) {
        EEPROM.write(Start, Write >> 8);
        EEPROM.write(Start+1, Write & 0xFF);
        EEPROM.commit();
}

float EepromReadFloat(int Start) {
        float Result;
        EEPROM.get(Start, Result);
        return(Result);
}

void EepromWriteFloat(float Write, int Start) {
        EEPROM.put(Start, Write);
        EEPROM.commit();
}

IPAddress EepromReadIP(int Start) {
        IPAddress Result;
        for (int i = 0; i < 4; i++) {
                Result[i] = EEPROM.read(Start + i);
        }
        return(Result);
}

void EepromWriteIP(IPAddress Write, int Start) {
        for (int i = 0; i < 4; i++) {
                EEPROM.write(Start+i, Write[i]);
        }
        EEPROM.commit();
}
