// P8355 ROM Dumper
// ~CE1, CLK, and IO/~M should be tied to GND
// CE2, Pin5(NC), ~IOR, and ~IOW should be tied to VCC(5v)
// Ports A and B can be left floating or tied to VCC

const int ad0 = 2;
const int ad1 = 3;
const int ad2 = 4;
const int ad3 = 5;
const int ad4 = 6;
const int ad5 = 7;
const int ad6 = 8;
const int ad7 = 9;
const int a8 = A0;
const int a9 = A1;
const int a10 = A2;

const int rst = 12;
const int rd = 11;
const int ale = 10;
const int startButton = 13;

int address = -1; //Prevents an off-by-one error
int go = 0;
int running = 0;

int readData(int addr) {
  //set the MUX'd address pins to OUTPUT
  pinMode(ad0, OUTPUT);
  pinMode(ad1, OUTPUT);
  pinMode(ad2, OUTPUT);
  pinMode(ad3, OUTPUT);
  pinMode(ad4, OUTPUT);
  pinMode(ad5, OUTPUT);
  pinMode(ad6, OUTPUT);
  pinMode(ad7, OUTPUT);

  //set the address
  digitalWrite(ad0,  addr &  1);
  digitalWrite(ad1, (addr >> 1)  & 1);
  digitalWrite(ad2, (addr >> 2)  & 1);
  digitalWrite(ad3, (addr >> 3)  & 1);
  digitalWrite(ad4, (addr >> 4)  & 1);
  digitalWrite(ad5, (addr >> 5)  & 1);
  digitalWrite(ad6, (addr >> 6)  & 1);
  digitalWrite(ad7, (addr >> 7)  & 1);
  digitalWrite(a8,  (addr >> 8)  & 1);
  digitalWrite(a9,  (addr >> 9)  & 1);
  digitalWrite(a10, (addr >> 10) & 1);
  
  //let the address pins settle
  delay(1);

  //latch the address (isn't the 8085 interface lovely...)
  digitalWrite(ale, HIGH);
  delay(1);
  digitalWrite(ale, LOW);
  delay(1);

  //set the Mux'd pins as INPUT
  pinMode(ad0, INPUT);
  pinMode(ad1, INPUT);
  pinMode(ad2, INPUT);
  pinMode(ad3, INPUT);
  pinMode(ad4, INPUT);
  pinMode(ad5, INPUT);
  pinMode(ad6, INPUT);
  pinMode(ad7, INPUT);

  //Read in the data!
  /*
   * Note: We should be checking the READY pin here with a clock signal
   * to ensure that we're adhering to the correct timing for reads.
   * However, we're reading slow enough that there's really no point
   * as (according to the datasheet) it should never take more than 450ns
   * for the data to be properly set up.
   * The reason we can get away with this is that the clock is actually
   * only used to yell at the 8085 that the ROM is _really_ slow.
   */
  digitalWrite(rd, LOW);
  delay(2); //Insurance.
  int ret = digitalRead(ad0);
  ret |= digitalRead(ad1) << 1;
  ret |= digitalRead(ad2) << 2;
  ret |= digitalRead(ad3) << 3;
  ret |= digitalRead(ad4) << 4;
  ret |= digitalRead(ad5) << 5;
  ret |= digitalRead(ad6) << 6;
  ret |= digitalRead(ad7) << 7;
  digitalWrite(rd, HIGH);
  delay(1);
  return(ret);
}

void setup() {
  Serial.begin(9600);
  //setup control pins
  pinMode(rst, OUTPUT);
  pinMode(ale, OUTPUT);
  pinMode(rd,  OUTPUT);
  pinMode(startButton, INPUT_PULLUP);

  //setup muliplexed pins
  pinMode(ad0, OUTPUT);
  pinMode(ad1, OUTPUT);
  pinMode(ad2, OUTPUT);
  pinMode(ad3, OUTPUT);
  pinMode(ad4, OUTPUT);
  pinMode(ad5, OUTPUT);
  pinMode(ad6, OUTPUT);
  pinMode(ad7, OUTPUT); 

  //setup non-multiplexed pins
  pinMode(a8, OUTPUT);
  pinMode(a9, OUTPUT);
  pinMode(a10,OUTPUT);

  //Reset the chip
  digitalWrite(ale, LOW);
  digitalWrite(rd, HIGH);
  digitalWrite(rst, HIGH);
  delay(100); //100ms wait while the chip sorts itself out
  digitalWrite(rst, LOW);
}

void loop() {
  if(go) {
    if(++address < 2048) {
      int data = readData(address);
      if(data < 16) Serial.print("0");
      Serial.print(data, HEX);
      if((address % 8) == 7) Serial.println();
    } else if (digitalRead(startButton)) { //make sure we're not holding down start...
      go = 0;
      address = -1; //prevent off-by-one
      delay(50); //switch debounce.
    }
  } else {
    int tmp = !digitalRead(startButton);
    delay(50); //switch debounce.
    if (tmp == !digitalRead(startButton)) {
      digitalWrite(rst, HIGH);
      delay(100); //100ms wait while the chip sorts itself out
      digitalWrite(rst, LOW);
      go = 1;
    }
  }
  delay(1);
}
