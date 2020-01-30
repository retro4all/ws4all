#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wsrom_good.h"
#include "unzip.h"

struct { int crc; const char *name; } WSMyDat[WS_DATROMS]=
{
	{0xb18cdc0d, "Alchemist Marie & Elie - Futari no Atelier (J) [!]"},
	{0xd7a0ab74, "Anaza Hebun - Memory of Those Days (J) [!]"},
	{0x425eb893, "Anchor Field Z (J) [M]"},
	{0xd022f6fd, "Anchor Field Z (J) [M][f1]"},
	{0xe2317345, "Arc The Lad - Kijin Fukkatsu (J)"},
	{0xc11639af, "Armored Unit (J) [M]"},
	{0x1b73df20, "Armored Unit (J) [M][f1]"},
	{0x04366d92, "Armored Unit (J) [M][o1]"},
	{0x392dd813, "Bakusou Dekatora Densetsu (J) [M][!]"},
	{0x09590272, "Bank Test by Sir Dragoon (PD)"},
	{0x8ba49dab, "Battle Spirit Digimon Frontier (J) [!]"},
	{0x324622c9, "Beat Mania (J) [M][!]"},
	{0x99027238, "Blue Wing Blitz (J)"},
	{0x99027238, "Blue Wing Blitz (J)"},
	{0x7da6acb9, "Bokan Densetsu Buta Mo O Daterya Doronbou (J) [M]"},
	{0xb25a0635, "Buffers Evolution (J) [M]"},
	{0x7f3a14c0, "Card Captor Sakura - Sakura to Fushigi na Clow Card (J) [M][!]"},
	{0x3d6951ca, "Chaos Demo V1.0 by Charles Doty (PD)"},
	{0xc06c68f6, "Chaos Demo V1.1 by Charles Doty (PD)"},
	{0x86f6a2a6, "Chaos Demo V2.0 by Charles Doty (PD)"},
	{0xad2b1d5a, "Chaos Demo V2.1 by Charles Doty (PD)"},
	{0x27b3cc18, "Chaos Gear - Michibi Kareshi Mono (J) [M][!]"},
	{0x56e2c069, "Cho Denki Card Battle Yofu Makai Kikuchi Shugo (J) [M][!]"},
	{0xccaa4853, "Chocobo no Fushigi na Dungeon (J) [M]"},
	{0x7fff2520, "Chou Aniki - Otoko no Tamafuda (J) [M]"},
	{0x62d419cd, "Choujikuu Yousai Macross - True Love Song (J) [M][!]"},
	{0xa9936941, "Clock Tower (J) [M][!]"},
	{0x4e8909d1, "Clock Tower (J) [M][o1]"},
	{0x8d96c62e, "Color Bars Demo by Sir Dragoon (PD)"},
	{0x22c786a0, "Color Scrolling Demo by Charles Doty (PD)"},
	{0x318f11ad, "Color Test Demo by Sir Dragoon (PD)"},
	{0xc995274b, "Crazy Climber (J) [M][!]"},
	{0x0cb57376, "Crazy Climber (J) [M][o1]"},
	{0xb1caec06, "D's Garage 21 Koubo Game - Tane wo Maku Tori (J) [M][!]"},
	{0xeba41a7f, "D's Garage 21 Koubo Game - Tane wo Maku Tori (J) [M][f1]"},
	{0x12ae932c, "Dark Eyes - Battle Gate (J) [!]"},
	{0xb62e5dd0, "Densha de Go! (J) [M]"},
	{0xaf9d8f42, "Densha de Go! 2 (J) [M]"},
	{0x1887389c, "Dicing Knight Period (J) [o1]"},
	{0x6e9dd148, "Digimon Adventure - Anode Tamer (J) [M]"},
	{0x42ad238b, "Digimon Adventure - Cathode Tamer (J) [M]"},
	{0x4d28637e, "Digimon Adventure 02 - D1 Tamers (J) [!]"},
	{0x4d28637e, "Digimon Adventure 02 - D1 Tamers (J) [!]"},
	{0x6ededaf8, "Digimon Adventure 02 - Tag Tamers (J) [M][!]"},
	{0x94ea6ffc, "Digimon Digital Monsters (As) [M]"},
	{0xab59de2e, "Digimon Digital Monsters (J) [M]"},
	{0x77689273, "Digimon Digital Monsters - Anode & Cathode Tamer - Veedramon Version (As) [!]"},
	{0x77689273, "Digimon Digital Monsters - Anode & Cathode Tamer - Veedramon Version (As) [!]"},
	{0xfaaefbcb, "Digimon Digital Monsters - D Project (J)"},
	{0x603cb5e6, "Digimon Digital Monsters for WonderSwanColor (J) [!]"},
	{0x0b03ccde, "Digimon Digital Monsters for WonderSwanColor (J) [f1]"},
	{0xc927bfcb, "Digimon Tamers - Battle Spirit (J) [!]"},
	{0xc927bfcb, "Digimon Tamers - Battle Spirit (J) [!]"},
	{0x6caad4a2, "Digimon Tamers - Battle Spirit V1.5 (J) [!]"},
	{0xf1f4d41f, "Digimon Tamers - Brave Tamer (J) [!]"},
	{0x7b4eb80d, "Digimon Tamers - Brave Tamer (J) [b1]"},
	{0xfa92418d, "Digimon Tamers - Digimon Medley (J) [!]"},
	{0x224f688a, "Digimon Tamers - Digimon Medley (J) [f1]"},
	{0xb5e675ae, "Digital Partner (J) [M][!]"},
	{0x89e7d950, "Dokodemo Hamster (J) [M][!]"},
	{0x93e19f13, "Dokodemo Hamster 3 - O Dekake Saffron (J)"},
	{0x067238d1, "Dragonball (J)"},
	{0x4e467626, "Engacho! (J) [M][!]"},
	{0x3dcb48bd, "Engacho! (J) [M][f1]"},
	{0x8922dd0b, "Fever Sankyo - Koushiki Pachinko Simulation (J) [M]"},
	{0xa83eabfd, "Fever Sankyo - Koushiki Pachinko Simulation (J) [M][f1]"},
	{0xb7243e88, "Final Fantasy (J) [!]"},
	{0xdf94a149, "Final Fantasy (J) [T+Eng0.91_Kalas]"},
	{0xf09e752f, "Final Fantasy II (J) [!]"},
	{0x89635cad, "Final Fantasy II (J) [T+Eng010]"},
	{0x15ec0525, "Final Fantasy II (J) [T+Eng097_RPGOne]"},
	{0x6045df59, "Final Fantasy II (J) [T-Eng095_RPGOne]"},
	{0xf699d6d1, "Final Fantasy IV (J) [!]"},
	{0xce2f1a1d, "Final Lap 2000 (J) [M][!]"},
	{0x459afb1d, "Final Lap 2000 (J) [M][f1]"},
	{0xb07e6a56, "Final Lap Special (J) [!]"},
	{0xc940c482, "Final Lap Special (J) [f1]"},
	{0xf246a68e, "Fire Pro Wrestling (J) [M]"},
	{0xa1fb16fc, "Fishing Freaks - Bass Rise (J) [M]"},
	{0x69d8d433, "Flash - Koibito Kun (J) [!]"},
	{0x03a2288c, "Flash - Koibito Kun (J) [f1]"},
	{0x8e120b5a, "From TV Animation - One Piece - Chopper no Daibouken (J) [!]"},
	{0xf2e362b8, "From TV Animation - One Piece - Mezase Kaizoku Ou (J) [M]"},
	{0xbe3dde09, "From TV Animation - One Piece - Mezase Kaizoku Ou (J) [M][f1]"},
	{0x427056c4, "From TV Animation - One Piece - Niji no Shima Densetsu (J) [!]"},
	{0x205503c3, "From TV Animation - One Piece - Treasure Wars (J) [!]"},
	{0x0d1048f0, "From TV Animation - One Piece - Treasure Wars 2 - Buggy Land e Youkoso (J) [!]"},
	{0x2f9e2560, "Front Mission (J)"},
	{0xa193458c, "Ganso Jajamaru Kun (J) [M][!]"},
	{0x8a8b827c, "Gekitou Crash Gear Turbo Gear Champion League (J) [!]"},
	{0x446e2581, "Gensou Maden Saiyuuki Retribution (J) [!]"},
	{0x67cba5c9, "Gensou Maden Saiyuuki Retribution (J) [f1]"},
	{0xeaadc794, "Girl Demo by Dox (PD)"},
	{0x8fcd6d2c, "Glocal Hexcite (J) [M][!]"},
	{0x4aed3911, "Glocal Hexcite (J) [M][o1]"},
	{0xbc944a98, "Golden Axe (J) [!]"},
	{0x6887257f, "Golden Axe (J) [t1]"},
	{0x6cd1411e, "Gomoku Narabe & Reversi Touryuumon (J) [M]"},
	{0x00137e86, "Gorakuoh TANGO! (J) [M]"},
	{0x1485ab1d, "Gorakuoh TANGO! (J) [M][f1]"},
	{0xaf24f95c, "Gransta Chronicle (J) [!]"},
	{0x9750bc2a, "Guilty Gear Petit (J)"},
	{0xd7a12bd5, "Guilty Gear Petit 2 (J) [!]"},
	{0x30dd9cee, "Guilty Gear Petit 2 (J) [t1]"},
	{0xa1656bbb, "GunPey (J) [M]"},
	{0x55df2cbb, "GunPey (J) [M][f1]"},
	{0x0c9cb12c, "GunPey Ex (J) [!]"},
	{0xa97dccd9, "GunPey Ex (J) [f1]"},
	{0x8e027330, "Hanafuda Shiyou Yo (J) [M][!]"},
	{0x4b22270d, "Hanafuda Shiyou Yo (J) [M][o1]"},
	{0x31e09bed, "Hanjyuku Hero - Aah Sekai yo Hanjuku Nare (J) [!]"},
	{0xaa525c04, "Harobots (J) [M][!]"},
	{0x7a29e9a6, "Hataraku Chocobo (J) [!]"},
	{0xf207546c, "Hello World by Sir Dragoon (PD)"},
	{0xa487b7a8, "Hunter X Hunter - Greed Island (J)"},
	{0x70aa800e, "Hunter X Hunter - Ichi O Tsugu Mono (J) [M][!]"},
	{0x088db40c, "Hunter X Hunter - Ichi O Tsugu Mono (J) [M][f1]"},
	{0x9402bca9, "Hunter X Hunter - Michibi Kareshi Mono (J) [!]"},
	{0x92bb2e53, "Hunter X Hunter - Michibi Kareshi Mono (J) [f1]"},
	{0xd0b20c5a, "Hunter X Hunter - Sorezore no Ketsui (J) [!]"},
	{0x0f1d6710, "Hunter X Hunter - Sorezore no Ketsui (J) [f1]"},
	{0x0c23d551, "Inu Yasha (J) [!]"},
	{0x3ff5791f, "Inu Yasha Fuu'un Emaki (J)"},
	{0x44453234, "Inu Yasha Kagome no Yumenikki (J) [!]"},
	{0xca3f0b00, "Itou Jun Ni Uzumaki Noroi Simulation (J) [M][!]"},
	{0x700f4c84, "Itou Jun Ni Uzumaki Noroi Simulation (J) [M][f1]"},
	{0x85c4c9a1, "Judgement Silversword - Rebirth Edition (J) (REV.SC21) [o1]"},
	{0xaa13e9de, "Judgement Silversword - Rebirth Edition (J) [o1]"},
	{0x4d103236, "Kakuto Ryori Densetsu Bistro Recipe - Wonder Battle Hen (J) [M]"},
	{0x80c6ef12, "Kaze no Klonoa - Moonlight Museum (J) [M]"},
	{0x411da7a4, "Kaze no Klonoa - Moonlight Museum (J) [M][t1]"},
	{0x945fc213, "Keiba Yosou Shien Shinkaron (J) [M][!]"},
	{0x517f962e, "Keiba Yosou Shien Shinkaron (J) [M][o1]"},
	{0xd4061551, "Kidou Senshi Gundam Giren no Yabou Tokubetsuhen Aoki no Hasha (J) [!]"},
	{0xb338cae2, "Kidou Senshi Gundam Seed (J) [T+Chi_amuro]"},
	{0xbce15137, "Kidou Senshi Gundam Seed (J)"},
	{0x08c0247b, "Kidou Senshi Gundam Vol.1 -Side 7- (J) [!]"},
	{0xa9f5bf54, "Kidou Senshi Gundum Vol.3 -A BAOA QU- (J) [!]"},
	{0x462f9275, "Kinniku Man Second Generations Dream Tag Match (J) [!]"},
	{0x625cc49c, "Kinnikuman Nisei Choujin Seisenshi (J) [b1]"},
	{0x6142fd9d, "Kinnikuman Nisei Choujin Seisenshi (J)"},
	{0x3b4c60c6, "Kiss Yori... - Seaside Serenade (J) [M]"},
	{0x274719f5, "Kurupara! (J) [!]"},
	{0xecfbcb1d, "Kyoso Uma Ikusei Simulation Keiba (J) [M]"},
	{0x9e0854e2, "Langrisser Millenium WS - The Last Century (J) [M][!]"},
	{0x9b6177e3, "Last Alive (J) [f1]"},
	{0xda4479bf, "Last Alive (J)"},
	{0xcccaf8a1, "LastStand (J) [M]"},
	{0xe407cabf, "Lode Runner (J) [M][!]"},
	{0x21279e82, "Lode Runner (J) [M][o1]"},
	{0x637ada93, "Magical Drop (J) [M][!]"},
	{0xb096ae0a, "Magical Drop (J) [M][b1]"},
	{0xe7e7fd4c, "Mahjong Touryuumon (J) [M][!]"},
	{0x1b6f5f30, "Makai Toushi Sa-Ga (J) [!]"},
	{0x00b31fbb, "Makaimura (J) [M]"},
	{0xb6ee35c8, "Makaimura (J) [M][t1]"},
	{0x12fb8b28, "Medarot - Perfect Edition Kabuto Version (J) [M][!]"},
	{0x2b40745d, "Medarot - Perfect Edition Kuwagata Version (J) [M][!]"},
	{0x92fbf7fb, "Meitantei Conan - Majutsushi no Chousenjou! (J) [M][!]"},
	{0xe226863b, "Meitantei Conan - Nishi no Meitantei Saidai no Kiki! (J) [M]"},
	{0x9f6e3f8d, "Meitantei Conan - Yugure no Koujo (J) [!]"},
	{0x8e123373, "Memories of Festa (J) [!]"},
	{0xa3ead689, "Meta Communication Therapy nee Kiite! (J) [M][!]"},
	{0xd75effc2, "Mikeneko Holme's Ghost Panic (J) [!]"},
	{0x4e9020b3, "Mikeneko Holme's Ghost Panic (J) [f1]"},
	{0x9baac7bb, "Mingle Magnet (J) [M]"},
	{0x39a1391a, "Mobile Suit Gundam - Volume 2 - JABURO (J) [!]"},
	{0x53b9fef8, "Mobile Suit Gundam MSVS (J) [M][!]"},
	{0xeb995e86, "Moero !! Pro Yakyu Rookies (J) [M]"},
	{0xb159c30d, "Morita Shougi (J) [M][!]"},
	{0xd675c265, "Morita Shougi (J) [M][o1]"},
	{0x5555d95c, "Mr. Driller (J) [!]"},
	{0x8ce4652f, "Namco Super Wars (J) [!]"},
	{0x6960b010, "Naruto (J) [b1]"},
	{0x71556e6a, "Naruto (J)"},
	{0x4ed8820c, "Nazo Ou Pocket (J) [M]"},
	{0x8d176482, "Nazo Ou Pocket (J) [M][f1]"},
	{0xbf8d9212, "Neon Genesis Evangelion Shito Ikusei (J) [M][!]"},
	{0xb5dbcf12, "Nice On (J) [M][!]"},
	{0x7638a3db, "Nice On (J) [M][f1]"},
	{0xc1734309, "Nobunaga no Yabo (J) [M][!]"},
	{0x04531734, "Nobunaga no Yabo (J) [M][o1]"},
	{0xf8a1dd2b, "One Piece - Grand Battle Swan Colloseum (J)"},
	{0x631bd97a, "Ouchan no Oekaki Logic (J) [M]"},
	{0xa16d1d16, "Ouchan no Oekaki Logic (J) [M][f1]"},
	{0x35361250, "Pocket Fighter (J) [M]"},
	{0xd3092e88, "Pocket no Naka no Doraemon (J) [f1]"},
	{0x2b61bb2b, "Pocket no Naka no Doraemon (J)"},
	{0x90f7e6d6, "Pro Mahjong Kiwame (J) [M][!]"},
	{0x5a41a7ba, "Puyo Puyo Tsu (J) [M][!]"},
	{0x302499b9, "Puzzle Bobble (J) [M]"},
	{0x6a05c838, "Puzzle Bobble (J) [M][f1]"},
	{0x15c4552e, "RUN=DIM Return to Earth (J) [!]"},
	{0x8f8608ad, "Rainbow Islands - Putty's Party (J) [M]"},
	{0xfe4ff701, "Raku Jongg (J) [!]"},
	{0x76309525, "Raku Jongg (J) [f1]"},
	{0x812b720d, "Rekishi Simulation Sangokushi II (J) [M]"},
	{0x60706555, "Rhyme Rider Kerorikan (J) [!]"},
	{0x14adbd4b, "Ring Infinity (J) [M][!]"},
	{0xac8db6a3, "Ring Infinity (J) [M][f1]"},
	{0xbe6be690, "Robot Works (As) [M]"},
	{0x6adf0e32, "Robot Works (J) [M][!]"},
	{0xcd206a9e, "Rockman & Forte - Mirai Kara no Chousen Sha (J) [M][!]"},
	{0x1f10ca75, "Rockman EXE N1 Battle (J)"},
	{0xa406c9e6, "Rockman EXE WS (J) [b1]"},
	{0x658c4b98, "Rockman EXE WS (J)"},
	{0x9c98d97d, "Romancing Saga (J) [!]"},
	{0xae83f873, "SD Gundam - Emotional Jam (J) [M][!]"},
	{0xf0acda5c, "SD Gundam - Operation U.C. (J)"},
	{0xc60e5162, "SD Gundam Eiyuuden - Eiyuuden Kishi Densetsu (J) [!]"},
	{0xc60e5162, "SD Gundam Eiyuuden - Eiyuuden Kishi Densetsu (J) [!]"},
	{0x18ecffb8, "SD Gundam Eiyuuden - Eiyuuden Musha Densetsu (J) [!]"},
	{0x18ecffb8, "SD Gundam Eiyuuden - Eiyuuden Musha Densetsu (J) [!]"},
	{0xe4eb3ab1, "SD Gundam G Generation - Gather Beat (J) [M]"},
	{0x4d21a347, "SD Gundam G Generation - Gather Beat 2 (J) [!]"},
	{0x6aca1f04, "SD Gundam G Generation - Mono-Eye Gundams (J)"},
	{0x21eb4c59, "SD Gundam Gashapon Senki - Episode 1 (J) [M][!]"},
	{0x6e8a792d, "Saint Seiya - Ougon Densetsu Hen Perfect Edition (J)"},
	{0xe385ee88, "Sangokushi (J) [M]"},
	{0x07a3dd46, "Senkaiden - TV Animation Senkaiden Houshin Engi Yori (J) [M]"},
	{0x9f35d00f, "Senkaiden Ni - TV Animation Senkaiden Houshin Engi Yori (J) [!]"},
	{0x9f35d00f, "Senkaiden Ni - TV Animation Senkaiden Houshin Engi Yori (J) [!]"},
	{0x301436ac, "Sennou Millenium (J) [M][!]"},
	{0x3ef0b0c4, "Sennou Millenium (J) [M][f1]"},
	{0x6c029674, "Shaman King Mirai E no Ishi (J) [b1]"},
	{0x34908cb4, "Shaman King Mirai E no Ishi (J)"},
	{0x1c489351, "Shanghai Pocket (J) [M]"},
	{0x5b76f901, "Shin Nihon Pro Wrestling Toukon Retsuden (J) [M]"},
	{0x6021d9cc, "Shin Nihon Pro Wrestling Toukon Retsuden (J) [M][f1]"},
	{0xfff1c0d6, "Shogi Touryuumon (J) [M][!]"},
	{0x3ad194eb, "Shogi Touryuumon (J) [M][o1]"},
	{0x8655269e, "Side Pocket (J) [M][!]"},
	{0xf00a0330, "Slither Link (J) [M][!]"},
	{0xf6e6fcee, "Slither Link (J) [M][f1]"},
	{0x352a570d, "Slither Link (J) [M][o1]"},
	{0xeaf7f3d9, "Slither Link (J) [M][o1][f1]"},
	{0xd7438e58, "Soccer Yarou! - Challenge the World (J) [M]"},
	{0x1263da65, "Soccer Yarou! - Challenge the World (J) [M][a1]"},
	{0x0e467d97, "Soroban Gu (J) [!]"},
	{0xf016dfe1, "Sotsugyou (J) [M]"},
	{0x8d83014f, "Space Invaders (J) [M][!]"},
	{0x138d1018, "Star Hearts (J) [!]"},
	{0x9874e9a2, "Star Hearts Taikenban (J) [!]"},
	{0x16e0d929, "Super Robot Taisen Compact (J) [!]"},
	{0x7021d54f, "Super Robot Taisen Compact (J) [M][!]"},
	{0x782877bc, "Super Robot Taisen Compact 2 - Dai Ichibu - Chijou Gekidou Hen (J) [M]"},
	{0xecbb46de, "Super Robot Taisen Compact 2 - Dai Nibu - Uchuu Gekishin Hen (J) [M][!]"},
	{0x0b0f8981, "Super Robot Taisen Compact 2 - Dai Sanbu - Ginga Kessen Hen (J) [M]"},
	{0x6918c824, "Super Robot Taisen Compact 3 (J) (V1.5)"},
	{0x188ca644, "Super Robot Taisen Compact 3 (J) (V1.6)"},
	{0x8b74f59a, "Tanjou - Debut (J) [M][!]"},
	{0xa5643aa3, "Tare Pan no Gunpei (J) [M][!]"},
	{0xd387c363, "Tare Pan no Gunpei (J) [M][f1]"},
	{0x2fcf1443, "Tare Pan no Gunpei (J) [M][f2]"},
	{0xe7c608e5, "Tekken Card Challenge (J) [M][!]"},
	{0xef5b6b82, "Terrors (J) [M]"},
	{0x9bd8f08c, "Terrors 2 (J) [!]"},
	{0xad9d62fe, "Terrors 2 (J) [f1]"},
	{0x7b0caea9, "Tetris (J) [!]"},
	{0x6f304dca, "Tetsujin 28 Gou (J) [M]"},
	{0x82a07866, "Tetsujin 28 Gou (J) [M][f1]"},
	{0x44b3e67c, "Tetsuman (J) [M][!]"},
	{0x91117d1b, "Tokyo Majin Gakuen Fuju Fuuroku (J) [M][!]"},
	{0x47659b2c, "Tonpusou (J) [!]"},
	{0xc31064f6, "Tonpusou (J) [f1]"},
	{0x64aaf392, "Trump Collection 2 (J) [M][!]"},
	{0x07a3b862, "Trump Collection Bottom Up Teki Trump Seikatsu (J) [M][!]"},
	{0xc283ec5f, "Trump Collection Bottom Up Teki Trump Seikatsu (J) [M][o1]"},
	{0x0d5171f0, "Turntablist - DJ Battle (J) [M][!]"},
	{0xa07482ac, "Turntablist - DJ Battle (J) [M][f1]"},
	{0x5793bdda, "Uchuu Senkan Yamato (J) [!]"},
	{0xd56b4fd8, "Uchuu Senkan Yamato (J) [b1]"},
	{0xde2208ab, "Ultraman - Hikari no Kuni no Shisha (J)"},
	{0x86b56511, "Umizuri Ni Ikou (J) [M][!]"},
	{0x812020ef, "Uzumaki - Denshi Kaiki Hen (J) [M][!]"},
	{0x89cbaa7c, "Uzumaki - Denshi Kaiki Hen (J) [M][f1]"},
	{0x8fc9e145, "Vaitz Blade (J) [M][!]"},
	{0x1860b655, "Wasabi Produce - Street Dancer (J) [M][!]"},
	{0xd9401f0a, "Wild Card (J) [!]"},
	{0xe14e9d36, "With You Mitsumete Itai (J) [!]"},
	{0xa44b167e, "With You Mitsumete Itai (J) [f1]"},
	{0x15e55706, "Wizardry - Scenario 1 - Kyounou no Shiren Jou (J) [!]"},
	{0x15e55706, "Wizardry - Scenario 1 - Kyounou no Shiren Jou (J) [!]"},
	{0x12f10b27, "Wonder Classic (J) [!]"},
	{0x12f10b27, "Wonder Classic (J) [!]"},
	{0xe252919d, "Wonder Stadium '99 (J) [M]"},
	{0x23bc0309, "Wonder Stadium (J) [M]"},
	{0x2ab9852d, "WonderSwan Handy Sonar (J) [M]"},
	{0x7e0d787b, "WonderSwan Register Diagnostic by Zalas (PD)"},
	{0xd93ab792, "Wondersnake 2 by Dox (PD)"},
	{0xe529d0c1, "Wondersnake Beta by Dox (PD)"},
	{0xa0591f00, "Wondersnake Final Sep 11 2001 by Dox & Kojote (PD)"},
	{0x66b617d5, "X - Card of Fate (J) [!]"},
	{0x25e2ba75, "XI (Sai) Little (J)"},
	{0x2460b45a, "Yakusoku no Chi Riviera (J)"},
};


int WS_DAT_LookFor(int CRC32)
{
 int i;

 for(i=0;i<WS_DATROMS;i++) if(WSMyDat[i].crc==CRC32) return i;

 return -1;
}

char *WS_DAT_getname(int no)
{
   return((char *)WSMyDat[no].name);
}


int get_crc (char * filename) {
  unsigned int crc;
#ifdef DEBUG
  unsigned long t_inicial = timerRead();
#endif

  if (check_zip (filename)) {
    //int size, skip_header;
    char romfile[0x100];
    unz_file_info fi;
    unzFile zf = unzOpen(filename);
    if(!zf) return 0;
    
    if(unzGoToFirstFile(zf) != UNZ_OK)
      return 0;
    
    if(unzGetCurrentFileInfo(zf, &fi, romfile, sizeof(romfile), NULL, 0, NULL, 0) != UNZ_OK)
      return 0;

    crc = fi.crc;
  }
  else {
    FILE *fd = NULL;
    unsigned char * file;    
    int size;

    fd = fopen(filename, "rb");
    if(!fd) return 0;
    
    /* Seek to end of file, and get size */
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    
    file = (unsigned char*) malloc(size);
    memset (file, 0, size);
    if(!file) 
	{
		fclose(fd);
		return 0;
	}
    fread(file, size, 1, fd);
    crc = crc32(0L, file, size);
    fclose(fd);
    free (file);
  }
#ifdef DEBUG
  printf ("Tiempo requerido en CRC: %d\n", timerRead()-t_inicial);
#endif
  return crc;
}
