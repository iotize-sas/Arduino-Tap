# How to IoTize your Arduino board

This application note shows how to add wireless connectivity to an Arduino application by plugging a TapNLink. 

Only a few minutes are required to iotize a Cortex-M based emmbedded application. But with most of Arduino boards, Integration is made easy thanks to the library concept. 

## Supported Arduino hardware

Arduino is a family based on different architecture. 
Practically, TapNLink supports any microcontroller and any architecture.  But depending on the board, the way to add a TapNLink module could slightly vary.
The main criteria are the voltage of the I/O pins, and the core. 
We have tested here on 3 popular boards: 

Model	Processor	IOs  Voltage
Uno	AVR (ATmega328)	5V
Due	Cortex-M (ATSAM3X8E)	3V
Mega	AVR (ATmega2560)	5V


| Model | Processor | IOs Voltage | 
|:-----:|:---------:|:------------|
| Uno	| AVR (ATmega328)	| 5V |
| Due	| Cortex-M (ATSAM3X8E)	| 3V |
| Mega	| AVR (ATmega2560)	| 5V |

### Core dependancy

We had to adapt slightly the interface: 
 - For the  Cortex-M based boards, we can use either the S3P protocol or the debug protocol (SWD). SWD is managed by the hardware of the core and is very simple to be use. It offers: 
        - the ability to start immediatly, without modifying the firmware,
        - various advanced features such as the update of the target (Arduino) firmware from a mobile.

S3P provides a better  securit, but security is not always the most important criteria for a Proof of Concept. Therefore, SWD would be prefer when available (e.g. for the Cortex-M based processors). 

 - for the other processors  (and potentially for the Cortex-M base as well), the TAP library must be added. This document describes how to use this library. 
 
 ### I/O voltage issue

 For processors with 5V digital pins, we have to adapt the voltage levels. Several solutions are possible, but simply insert a resistor for each digital signal. 
 The figure below shows the connection between Arduino-Uno and TapNLink:

 <img src="res/Arduino-to-Tap.png" alt="Wire connection to TapNLink" style="max-width: 300px; border: 1px solid gray;">

Whatever the protocol (SWD/S3P), we need 4 wires to make the Arduino board communicating with TapNLink:
 

Que l’on soit en SWD ou en S3P, et quelle que soit la carte Arduino, 4 fils sont nécessaires pour porter les signaux suivants :
Type	Name	Description
Power	Vcc	MUST BE 3V or 3.3V
	Gnd	Ground
Digital	Clock	Must be an interrupt (or could be SWD-CLK for Cortex-M devices)
	IO	

Donc deux pour fils pour l’alimentation (GND et Vcc3.3) et deux signaux logiques : une clock et un signal de données auxquels on peut optionnellement ajouter le signal de reset si l’on souhaite que TapNLink soit capable de resetter à la carte applicative. 
ATTENTION :  Avec une carte 5V, c’est bien l’alimentation 3.3V qu’il convient d’apporter le module TapNLink. DO NOT CONNECT 5V, it will destroy your Tap!
Connexion avec une carte 5V (UNO, MEGA, ..)
A nouveau, bien choisir son alimentation : il faut connecter la source 3.3V de la carte et non l’alimentation générale 5V. Mais il convient aussi d’adapter les signaux logiques car les pattes du TapNLink ne supportent pas non plus une tension de 5V. Pour cela, le plus simple consiste à intercaler une résistance 1ko (mille ohm) en série entre les deux cartes. La chute de tension sera suffisante pour limiter le courant et ne pas endommager TapNLink lorsqu’un signal haut est sorti depuis la carte Arduino. Elle sera aussi suffisamment faible pour ne pas ralentir les fronts de montée, et assurer des niveaux de tension convenable de part et d’autre. 
Le schéma suivant résume ces branchements : 
 
Dans le cas particulier de l’Arduino Uno, le signal IO peut être connecté sur n’importe quelle entrée sortie digitale alors que le signal CLK doit être connecté sur une patte d’interruption externe. Deux pattes seulement sont possibles : 2 ou 3. 
Dans l’exemple fourni avec la bibliothèque, CLK est relié à la patte 3 alors que IO est relié à la patte 5. 
Connexion avec une carte à base de Cortex-M (Due)
Si le protocole SWD est utilisé, le plus simple est de le connecter directement au port de debug du processeur : 
Configuration de l’IDE-Arduino
Fichier de sortie (ELF)
Pour IoTize Studio, nous avons besoin pour chaque projet du fichier exécutable (associé au suffixe ‘.ELF’) généré par l’IDE Arduino. Or l’IDE considère ce fichier comme un fichier de travail qu’il place par défaut dans un répertoire temporaire, peu accessible. On peut cependant modifier cet emplacement temporaire et le remplacer par un endroit plus accessible.  
Pour cela, il faut ouvrir le fichier ‘Preferences.txt’ Redirection par ‘préférences’ : 
Menu Fichier | Preferences | ‘More preferences can be edited…’
Avant d’éditer ce fichier, il faut fermer Arduino IDE (qui l’écrase à chaque fermeture) car l’on perdrait sinon les modifications. 
Ajouter une ligne ‘build.path=…’ en spécifiant après ‘=’ {sketchbook_directory\output}. Dans mon cas, mon répertoire ‘sketchbook’ est E:\Documents\Arduino :  
 
Sauver ce fichier, et lecoms fichiers générés par le compilateur GCC de l’IDE Arduino seront désormais placés dans ce répertoire, non loin de nos projets (sketch) Arduino. 
Installation de la bibliothèque TAP

Modification d’un projet existant
Structure des répertoires
Configuration du module
Installation d’IoTize Studio
Les choix de configuration
Sélection  et affichage de variables
Test
Monitoring local
Publication de l’Apps mobile
Depuis un mobile…
Pour aller plus loin…


