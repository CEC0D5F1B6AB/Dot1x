#define EAPOL_EAPPACKET 0
#define EAPOL_START 1
#define EAPOL_LOGOFF 2
#define EAPOL_KEY 3
#define EAP_REQUEST 1
#define EAP_RESPONSE 2
#define EAP_SUCCESS 3
#define EAP_FAILURE 4
#define EAP_TYPE_IDENTITY 1
#define EAP_TYPE_NOTIFICATION 2
#define EAP_TYPE_MD5 4
#define EAP_KEY_RC4 1
#define PKT_SIZE 2048
#define salt "zte142052"

#define max(a,b) ( ((a)>(b)) ? (a):(b) )
#define min(a,b) ( ((a)>(b)) ? (b):(a) )

#pragma pack(1)
struct eth
{
    unsigned char dest[6]; //Destination
    unsigned char source[6]; //Source
    unsigned short int proto; //Type
};

struct eapol
{
    unsigned char ver; //Version
    unsigned char type; //Type
    unsigned short int len; //Length
};

struct eap
{
    unsigned char code;//Code
    unsigned char id;//Id
    unsigned short int len; //Length
    unsigned char type; //Type
};

struct md5
{
    unsigned char len;//EAP-MD5 Value-Size
    unsigned char value[16];//EAP-MD5 Value
    unsigned char username[1];//Username
};

struct key
{
    unsigned char keytype;//Key Descriptor Type
    unsigned short int keylen; //Key Length
    unsigned char rc[8];//Replay Counter
    unsigned char keyiv[16];//Key IV
    unsigned char keyindex;//Key Index
    unsigned char keysignature[16];//Key Signature
    unsigned char key[16];//Key
};
#pragma pack()

int sockfd, status, e, count, count_aim;
unsigned char buf[PKT_SIZE], src_addr[6], des_addr[6] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03};
struct eth *eth = (struct eth *)buf;
struct eapol *eapol = (struct eapol *)(buf + sizeof(struct eth));
struct eap *eap = (struct eap *)(buf + sizeof(struct eth) + sizeof(struct eapol));
unsigned char *last = (unsigned char *)(buf + sizeof(struct eth) + sizeof(struct eapol) + sizeof(struct eap));
struct key *key = (struct key *)(buf + sizeof(struct eth) + sizeof(struct eapol));
struct md5 *md5 = (struct md5 *)(buf + sizeof(struct eth) + sizeof(struct eapol) + sizeof(struct eap));