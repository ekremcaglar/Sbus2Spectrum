typedef enum{
    S_IDLE,
    S_AA_RECEIVED,
    S_55_RECEIVED,
    S_CRC_RECEIVED
}state_t;


typedef struct{
    state_t state;
    uint8_t rx_buffer[64];
    uint8_t *rx_ptr;
    uint8_t *end_ptr;
    uint8_t tx_buffer[64];
    uint8_t *tx_ptr;
}state_machine_t;

typedef union{
    struct{
        uint8_t fades;
        uint8_t sys;
        uint16_t ch[7];
    };
    uint8_t buffer[16];
}spektrum_t;

typedef union{
    struct{
        uint16_t start1;
        uint16_t start2;
        uint8_t header;
        uint8_t data[22];
        uint8_t digital;
        uint8_t footer;
        uint16_t crc;
    };
    uint8_t buffer[31];
}desert_rotor_t;


uint16_t drotor_channels[16]={0};
state_machine_t uart_sm;


int state_machine_init(state_machine_t *usm){
    usm->state=S_IDLE;
    usm->rx_ptr=usm->rx_buffer;
    usm->end_ptr=usm->rx_buffer+31; //31
    memset(usm->rx_buffer,0,sizeof(usm->rx_buffer));
    return 0;
}

state_t state_machine_byte_received(state_machine_t *usm, uint8_t byte){
    switch(usm->state){
    case S_IDLE:
        if(0x55==byte){
            *usm->rx_ptr++=byte;
            usm->state=S_55_RECEIVED;
        }
        break;
    case S_55_RECEIVED:
        if(0xAA==byte){
            *usm->rx_ptr++=byte;
            usm->state=S_AA_RECEIVED;
        }else{
            usm->state=S_IDLE;
        }
        break;
        case S_AA_RECEIVED:
            *usm->rx_ptr++=byte;
            if(usm->rx_ptr == usm->end_ptr){
                usm->state=S_CRC_RECEIVED;
                usm->rx_ptr=usm->rx_buffer;
            }
    default:
        break;

    }
    return usm->state;
}

int state_machine_set_idle(state_machine_t *usm){
    usm->state=S_IDLE;
    usm->rx_ptr=usm->rx_buffer;
    return 0;
}


int spektrum_encode(state_machine_t *usm){
  uint16_t temp;
  drotor_decode((desert_rotor_t *)usm->rx_buffer);

  spektrum_t *spektrum= ((spektrum_t *)usm->tx_buffer);
  //temp=(spektrum->ch[i]>>8) + (spektrum->ch[i]<<8);
  spektrum->fades=0x00;
  spektrum->sys=0xA2;

  temp=(drotor_channels[0]&0x07FFu)+0x0800u;
  spektrum->ch[0]=((temp>>8) | (temp<<8));

  temp=(drotor_channels[1]&0x07FFu)+0x2800u;
  spektrum->ch[1]=((temp>>8) | (temp<<8));
  
  temp=(drotor_channels[2]&0x07FFu)+0x1000u;
  spektrum->ch[2]=((temp>>8) | (temp<<8));
  
  temp=(drotor_channels[3]&0x07FFu)+0x1800u;
  spektrum->ch[3]=((temp>>8) | (temp<<8));
  
  temp=(drotor_channels[4]&0x07FFu)+0x0000u;
  spektrum->ch[4]=((temp>>8) | (temp<<8));
  
  temp=(drotor_channels[5]&0x07FFu)+0x2000u;
  spektrum->ch[5]=((temp>>8) | (temp<<8));
  
  temp=(drotor_channels[6]&0x07FFu)+0x3000u;
  spektrum->ch[6]=((temp>>8) | (temp<<8));
}


int drotor_decode(const desert_rotor_t *in){
    int i,j,temp,n,div,rem;
    for(i=0;i<16;i++){
        temp=0;
        for(j=0;j<11;j++){
            n=11*i+j;
            div=n>>3;
            rem=n&0x7u;
            temp+=(((in->data[div]&(1<<rem))>>rem)<<j);
        }
        drotor_channels[i]=temp;
        //printf("%d\n",temp);
    }
    return 0;
}


void setup() {
  state_machine_init(&uart_sm);
    Serial.begin( 115200 );
}


void loop() {
  int ledPin = 13;
  if ( Serial.available() > 0 ) {
      if(S_CRC_RECEIVED == state_machine_byte_received(&uart_sm, Serial.read())){
        digitalWrite(ledPin, HIGH);
        spektrum_encode(&uart_sm);
        Serial.write(uart_sm.tx_buffer, sizeof(spektrum_t));
        state_machine_set_idle(&uart_sm);
        digitalWrite(ledPin, LOW);
      }
  }
}
