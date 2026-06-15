/**********************************************************************
	UI_QMENU.C

	Quake's menu system.
**********************************************************************/
#include "ui_local.h"

char *menu_normal_text[MNT_MAX];

/*
char *menu_normal_text[MNT_MAX] = 
{
NULL,								//	MNT_NONE

"ACCESSING VOYAGER LCARS . . .",	//	MNT_ACCESSINGLCARS,
"CLOSING VOYAGER LCARS . . .",		//	MNT_CLOSINGLCARS,

// Main Screen Headings
"ELITE FORCE : MAIN MENU",			//	MNT_MAINMENU_TITLE
"ELITE FORCE : QUIT MENU",			//	MNT_QUITMENU_TITLE
"ELITE FORCE : CREDITS",			//	MNT_CREDITSMENU_TITLE
"ELITE FORCE : SAVE GAME",			//	MNT_SAVEGAMEMENU_TITLE
"ELITE FORCE : NEW GAME",			//	MNT_NEWGAMEMENU_TITLE
"ELITE FORCE : LOAD GAME",			//	MNT_LOADGAMEMENU_TITLE
"ELITE FORCE : SETUP",				//	MNT_CONTROLSMENU_TITLE
"ELITE FORCE : EXPLORE VOYAGER",	//	MNT_EXPLORE_TITLE

// Voyager Crew Menu
"ELITE FORCE : VOYAGER CREW",		//	MNT_VOYAGERCREW_TITLE

"SENIOR STAFF",						//	MNT_SENIORSTAFF_DESC1,
"HAZARD TEAM ALPHA",				//	MNT_ALPHATEAM_DESC1
"HAZARD TEAM BETA",					//	MNT_BETATEAM_DESC1,

"CREW DATA",						//	MNT_CREWDATA,
"KATHRYN JANEWAY, CAPTAIN",			//	MNT_SENIOR1_DESC1,
"NAME : JANEWAY, KATHRYN",			//	MNT_SENIOR1_DESC2,
"RANK : CAPTAIN",					//	MNT_SENIOR1_DESC3,
"BORN : 2332",						//	MNT_SENIOR1_DESC4,
"PLACE OF BIRTH : INDIANA, U.S.A., EARTH",			//	MNT_SENIOR1_DESC5,
"ASSIGNMENT : CAPTAIN, U.S.S. VOYAGER NCC-74656",	//	MNT_SENIOR1_DESC6,
"ACHIEVEMENTS : DOCTORATE, QUANTUM COSMOLOGY",		//	MNT_SENIOR1_DESC7,
"UPON GRADUATION, ASSIGNED TO U.S.S. ICARUS, UNDER ADMIRAL OWEN PARIS. IN 2354,BRIEFLY TAKEN PRISONER WITH ADMIRAL PARIS BY CARDASSIANS, AIDED HIS ESCAPE. RECEIVED COMMENDATION FOR BRAVERY. SERVED AS SCIENCE OFFICER ON U.S.S. AL-BATANI UNDER OWEN PARIS.  LATER TRANSFERRED TO COMMAND DIVISION. IN 2370, TOOK COMMAND OF THE U.S.S. VOYAGER, 6 MONTH VENTURE INTO BETA QUADRANT. ON STARDATE 48307.5, VOYAGER WAS SWEPT INTO THE DELTA QUADRANT BY AN ALIEN ENTITY KNOWN AS THE CARETAKER. INTEGRATED MAQUIS INTO HER CREW AND ACCEPTED TWO DELTA QUADRANT NATIVES. JANEWAY, THE FEDERATIONS ONLY PRESENCE IN THE DELTA QUADRANT, MADE NUMEROUS FIRST CONTACTS WITH ALIEN LIFEFORMS. ENCOUNTERED Q ENTITIES IN 2372 AND 2373. IN 2374, JANEWAY ENCOUNTERED HOSTILE SPECIES 8472. TO COMBAT THEM SHE MADE HISTORIC FIRST FEDERATION ALLIANCE WITH THE BORG.",									//	MNT_SENIOR1_DESC8

"CHAKOTAY",																				//	MNT_SENIOR2_DESC1,
"NAME : CHAKOTAY",																		//	MNT_SENIOR2_DESC2,
"RANK : COMMANDER",																		//	MNT_SENIOR2_DESC3,
"ASSIGNMENT : FIRST OFFICER",															//	MNT_SENIOR2_DESC4,
"FAMILY : FATHER, KOLOPAK (DECEASED)",													//	MNT_SENIOR2_DESC5,
"INTERESTS : ARCHAEOLOGY",																//	MNT_SENIOR2_DESC6,
"Chakotay is of Native American descent and fiercely proud of it. But as a teenager, he hurt his father deeply by leaving his people to join Starfleet. His application to the Academy was sponsored by Captain Sulu. Chakotay came to realize the importance of his heritage and had his forehead tattooed in honor of his ancestors. After several years, he quit Starfleet and joined the Maquis to help in the defense of his home world against the Cardassians. He was appointed first officer of Voyager when both his Maquis vessel and Voyager were swept into the Delta Quadrant by an entity known as 'The Caretaker'.",	//	MNT_SENIOR2_DESC7,

"TUVOK",																				//	MNT_SENIOR3_DESC1,
"RACE : VULCAN",																		//	MNT_SENIOR3_DESC2,
"ASSIGNMENT : HEAD OF SECURITY",														//	MNT_SENIOR3_DESC3,
"FAMILY : WIFE, THREE SONS, ONE DAUGHTER",												//	MNT_SENIOR3_DESC4,
"INTERESTS : VULCAN LUTE",																//	MNT_SENIOR3_DESC5,
"ACHIEVEMENTS : TAUGHT AT STARFEET FOR 16 YEARS",										//	MNT_SENIOR3_DESC6,
"Forced into Starfleet by his parents in 2289, Tuvok found the multicultural environment difficult. After graduation, he served aboard the U.S.S. Excelsior under Captain Hikaru Sulu. He spoke out against Captain Spock's proposal of a Federation / Klingon alliance and shortly thereafter left Starfleet. But after raising his own family, Tuvok understood his parent's motives and in 2349 he returned to Starfleet. In 2371, Tuvok went undercover and infiltrated the Maquis where he served onboard Chakotay's ship. His official title on Voyager is Security Officer, but his unofficial role is that of advisor; many of the crew turn to him for counsel in times of need.",	//	MNT_SENIOR3_DESC7,

"B'ELANNA TORRES",																		//	MNT_SENIOR4_DESC1,
"NAME : TORRES, B'ELANNA",																//	MNT_SENIOR4_DESC2,
"RACE : KLINGON / HUMAN",																//	MNT_SENIOR4_DESC3,
"PARENTS : MOTHER - KLINGON, FATHER - HUMAN",											//	MNT_SENIOR4_DESC4,
"ASSIGNMENT : CHIEF OF ENGINEERING",													//	MNT_SENIOR4_DESC5,
"PLANET OF BIRTH : KESSIK IV",															//	MNT_SENIOR4_DESC6,
"Half-Klingon and Half-Human, Torres has difficulty controlling her temper, much of which can be attributed to her Klingon lineage. She attended Starfleet Academy and participated in the decathlon team. She left during the second year due to difficulties with discipline. But she so impressed Professor Chapman, an Academy teacher, he recommended she be accepted if she should ever seek to be re-admitted. She joined the Maquis and served under Chakotay. When the Maquis crew joined with Voyager's, Torres had a violent incident with Lt. Carey, her superior officer. Despite this incident, Torres was chosen to be the engineer of Voyager. She is currently having a relationship with Tom Paris, who aids her in accepting herself and her Klingon heritage.",	//	MNT_SENIOR4_DESC7,

"TOM PARIS",																			//	MNT_SENIOR5_DESC1,
"NAME : PARIS, THOMAS EUGENE",															//	MNT_SENIOR5_DESC2,
"RANK : ENSIGN",																		//	MNT_SENIOR5_DESC3,
"DEGREE : ASTROPHYSICS",																//	MNT_SENIOR5_DESC4,
"INTERESTS : 20th-CENTURY AMERICA",														//	MNT_SENIOR5_DESC5,
"ASSIGNMENT : CONN (FLIGHT CONTROLLER)",												//	MNT_SENIOR5_DESC6,
"ACHIEVEMENTS : FIRST HUMAN PILOT TO ACHIEVE WARP 10",									//	MNT_SENIOR5_DESC7,
"Paris's career in Starfleet was expected to be exemplary. His family contained a long line of Starfleet legends; his Great-Grandfather, Grandmother, Father and Aunt were all Admirals. While at the Academy, he performed excellently until an accident occurred causing the deaths of three crewmates. Fearing he would bring disgrace to the family name,he lied during the investigation, placing blame on one of the dead men. Later, he admitted to falsifying reports and was forced to leave Starfleet. Paris joined the Maquis but was caught on his first mission and imprisoned. In 2371, at the request of Captain Janeway, he was released after he agreed to help locate his former Maquis friends. After Voyager was swept to the Delta Quadrant, Janeway reinstated Paris's Starfleet commission and assigned him the ship's conn.",	//	MNT_SENIOR5_DESC8,

"HARRY KIM",																			//	MNT_SENIOR6_DESC1,
"NAME : KIM, HARRY",																	//	MNT_SENIOR6_DESC2,
"BORN : 2349",																			//	MNT_SENIOR6_DESC3,
"RANK : ENSIGN",																		//	MNT_SENIOR6_DESC4,
"ASSIGNMENT : OPS (OPERATIONS OFFICER)",												//	MNT_SENIOR6_DESC5,
"INTERESTS : CLARINET",																	//	MNT_SENIOR6_DESC6,
"Harry Kim graduated with honors from Starfleet Academy on Stardate 47918 and was posted to Voyager shortly afterwards. While at the Academy he was editor of the school newspaper. He plays the clarinet and spent a week's worth of his Voyager rations to replicate one. Family is very important to Kim; his biggest regret after the abduction of Voyager was the pain and sadness the new would bring to his parents. Being the only child of doting parents, Kim came to Voyager somewhat naïve, having experience little adversity in his life. But in the time he has spent as operations officer of Voyager, he has become a seasoned Starfleet veteran.",	//	MNT_SENIOR6_DESC7,

"DOCTOR",																				//	MNT_SENIOR7_DESC1,
"NAME : THE DOCTOR",																	//	MNT_SENIOR7_DESC2,
"ASSIGNMENT : Doctor",																	//	MNT_SENIOR7_DESC3,
"ACTIVATED : STARDATE 48308",															//	MNT_SENIOR7_DESC4,
"KNOWLEDGE BASE: SPANNING OVER 3,000 CULTURES",											//	MNT_SENIOR7_DESC5,
"INITIAL DATA BASE SIZE : 50 MILLION GIGAQUADS",										//	MNT_SENIOR7_DESC6,
"The Doctor is not really a person, but a holographic figure - an emergency medical program created by Starfleet. The primary program was developed at Jupiter station and was designed by Dr. Lewis Zimmerman. After the entire medical staff of the U.S.S. Voyager were killed in 2371, during the violent passage into the Delta Quadrant, the Doctor became the only source of medical treatment available. The Doctor is knowledgeable on over five million treatments with information from 2,000 medical references and the experiences of 47 physicians. The EMH (Emergency Medical Program) is capable of independent thought and, with the aid of an autonomous holo-emitter he wears, he is able to venture out from the sickbay on away missions.",		//	MNT_SENIOR7_DESC7

"SEVEN",																				//	MNT_SENIOR8_DESC1,
"NAME : SEVEN OF NINE",																	//	MNT_SENIOR8_DESC2,
"BORG DESIGNATION : TERTIARY ADJUNCT OF UNIMATRIX ZERO ONE",							//	MNT_SENIOR8_DESC3,
"ORIGINAL NAME : ANNIKA HANSEN",														//	MNT_SENIOR8_DESC4,
"BORN : 2348",																			//	MNT_SENIOR8_DESC5,
"SEPARATED FROM COLLECTIVE : 2374",														//	MNT_SENIOR8_DESC6,
"Seven of Nine was a Borg drone also known as Tertiary Adjunct of Unimatrix Zero One. She was originally a human female named Annika Hansen, born in 2348 at the Tendara Colony. Her parents were noted scientists who had departed for deep space aboard the starship Raven with their daughter Annika. The Raven crashed on a Class-M moon where her parents were presumed killed by Borg and Annika was assimilated. In 2347, Seven of Nine was assigned as liaison to Voyager when the Borg formed an alliance with Janeway in an attempt to defeat Species 8472. When her Borg Cube was destroyed, Seven stayed aboard Voyager. Although her return to human society has proven difficult, Seven of Nine is a valuable member of Voyager.",					//	MNT_SENIOR8_DESC7,

"NEELIX",																				//	MNT_SENIOR9_DESC1,
"NAME : NEELIX",																		//	MNT_SENIOR9_DESC2,
"RACE : TALAXIAN",																		//	MNT_SENIOR9_DESC3,
"BORN : RINAX, A MOON OF TALAX",														//	MNT_SENIOR9_DESC4,
"INTERESTS : ORCHID BREEDING",															//	MNT_SENIOR9_DESC5,
"PREVIOUS OCCUPATION: TRADER",															//	MNT_SENIOR9_DESC6,
"Neelix was originally a trader of junk and space debris, but he joined the Voyager crew in 2371 as a guide through the Delta Quadrant. A Jack-Of-All-Trades, Neelix also assumed the responsibility as ship's chief and converted part of the mess hall into a kitchen to help save energy used by food replicators. He is able to prepare meals with food grown in the hydroponics bay or gathered from planets Voyager visits. He is also self-appointed morale officer and has been called upon to act as a diplomat on several occasions when Voyager has encountered new species.",	//	MNT_SENIOR9_DESC7,


"LES FOSTER, LIEUTENANT",																//	MNT_HAZARD1_DESC1,
"NAME : FOSTER, LESTER WILLIAM",														//	MNT_HAZARD1_DESC2,
"RANK : LIEUTENANT",																	//	MNT_HAZARD1_DESC3,
"BORN : APRIL 20, 2339",																//	MNT_HAZARD1_DESC4,
"PLACE OF BIRTH : MELBOURNE, AUSTRALIA, EARTH",											//	MNT_HAZARD1_DESC5,
"ASSIGNMENT : HAZARD TEAM LEADER",														//	MNT_HAZARD1_DESC6,
"ACHIEVEMENTS : PURPLE HEART, DISTINGUISHED SERVICE MEDAL",								//	MNT_HAZARD1_DESC7,
"AS A CHILD LES FOSTER LIVED A TOUGH, SIMPLE LIFE, RAISED IN THE 'MANUALIST' LIFESTYLE OF HIS FAMILY. THEY BELIEVED THAT MODERN TECHNOLOGY MADE PEOPLE WEAK AND MANUAL LABOR WAS IMPORTANT TO CHARACTER BUILDING, HENCE THEIR MOTTO 'LABOR OMNIA VINCIT' ('LABOR CONQUERS ALL'). HE JOINED STARFLEET AT A YOUNG AGE AND WENT INTO SERVICE AS A MARINE, FIGHTING IN THE CARDASSIAN WARS. AFTERWARDS, FOSTER RETURNED TO STARFLEET FOR OFFICER TRAINING. TO HIS DISGUST MANY OF HIS FORMER SQUAD MATES JOINED THE MAQUIS TO ILEEGALLY FIGHT THE CARDASSIANS IN 2370. WHEN OFFERED THE VOYAGER ASSIGNMENT TO HELP CAPTURE THE TURNCOATS, FOSTER GLADLY SIGNED ON AS LEADER OF VOYAGER'S BETA SQUAD SECURITY TEAM. HE HAS CURRENTLY BEEN ASSIGNED AS LEADER OF THE NEWLY FORMED HAZARD TEAM.",//	MNT_HAZARD1_DESC8,

"ALEXANDER MUNRO, ENSIGN",																//	MNT_HAZARD2_DESC1,
"NAME : MUNRO, ALEXANDER",																//	MNT_HAZARD2_DESC2,
"RANK : ENSIGN",																		//	MNT_HAZARD2_DESC3,
"BORN : MAY 1, 2349",																	//	MNT_HAZARD2_DESC4,
"PLACE OF BIRTH : NEW CHICAGO, LUNA",													//	MNT_HAZARD2_DESC5,
"ASSIGNMENT : HAZARD TEAM, SECOND IN COMMAND",											//	MNT_HAZARD2_DESC6,
"ACHIEVEMENTS : MVP NEW CHICAGO HS RUGBY TEAM, 2365",									//	MNT_HAZARD2_DESC7,
"MUNRO WAS BORN ON EARTH'S MOON AT NEW CHICAGO NEAR LAKE ARMSTRONG. EARLY IN HIS CHILDHOOD HIS FATHER DIED, LEAVING YOUNG ALEX WITH A DEEP FEELINGS OF RESPONSIBLITY TOWARDS THE CARE OF HIS YOUNGER SIBLINGS. HE WAS CAPTAIN OF HIS HIGHSCHOOL RUGBY TEAM BUT WAS FORCED TO LEAVE FOR STARFLEET ACADEMY BEFORE THE CHAMPOINSHIP GAME. AT THE ACADEMY HE DID VERY WELL EXCEPT FOR A NOTABLE TROUBLE WITH DISCIPLINE WHICH GARNERED HIM SEVERAL DEMERITS. THE U.S.S. VOYAGER WAS HIS FIRST ASSIGNMENT AND HE HAS PROVEN HIMSELF TO BE A VALUABLE SECURITY OFFICER THOUGH HE HAS A TENDENCY TO TAKE RISKS WITHOUT CONSIDERING THE CONSEQUENCES. WHEN THE HAZARD TEAM WAS CREATED, HE WAS CHOSEN AS SECOND IN COMMAND.", //	MNT_HAZARD2_DESC8,

"AUSTIN CHANG, CREWMAN",																//	MNT_HAZARD3_DESC1,
"NAME : CHANG, AUSTIN MICHAEL",															//	MNT_HAZARD3_DESC2,
"RANK : CREWMAN",																		//	MNT_HAZARD3_DESC3,
"BORN : AUGUST 22, 2350",																//	MNT_HAZARD3_DESC4,
"PLACE OF BIRTH : TAYNOR-BAN-FAL, RIGEL XII",											//	MNT_HAZARD3_DESC5,
"ASSIGNMENT : HAZARD TEAM DEMOLITIONIST",												//	MNT_HAZARD3_DESC6,
"ACHIEVEMENTS : ENGINEERING DEGREE, SEC",												//	MNT_HAZARD3_DESC7,
"Austin is the son of a Human woman and a Rigellian man. His father was a scientist who studied warp field theory and was in constant demand throughout the Federation. The popularity of his father resulted in a childhood spent moving from one world to the next, never staying long enough to make close friends. He lived for a few formative years at the Keernai Colony on Chyribdis. The Keernai culture believed talking in public to be extremely rude and this social taboo caused Chang no end of trouble. He finally escaped this unstable childhood by enlisting in the Starfleet Engineering Corps where he excelled at demolition. Soon he went to Starfleet Academy where he trained in security. While there he met Alexander Munro and the two became best friends. Voyager was their first assignment. Austin's knowledge of explosives and demolition made him an ideal choice for the Hazard Team.",	//	MNT_HAZARD3_DESC8,

"KENDRICK BIESSMAN,CREWMAN",															//	MNT_HAZARD4_DESC1,
"NAME : BIESSMAN, KENDRICK JAMES",														//	MNT_HAZARD4_DESC2,
"RANK : CREWMAN (FORMER MAQUIS)",														//	MNT_HAZARD4_DESC3,
"BORN : JULY 17, 2349",																	//	MNT_HAZARD4_DESC4,
"PLACE OF BIRTH : ARES CITY, MARS",														//	MNT_HAZARD4_DESC5,
"ASSIGNMENT : WEAPONS SPECIALIST",														//	MNT_HAZARD4_DESC6,
"ACHIEVEMENTS : NAUSSICAN WRESTLING CHAMPIONSHIP 2368",									//	MNT_HAZARD4_DESC7,
"As a child Biessman lived in the enhanced gravity dome-cities of Mars. Unlike Mars 'natives' who lived outside the domes in the planet's natural low gravity, he developed the necessary strength to live on worlds with higher gravity. But having seen what living in low gravity can do to a one's body, Kendrick became an exercise enthusiast, giving him a muscular physique. His father was one of the Utopia Planetia Fleet Yard workers who helped build the U.S.S. Enterprise-D. It was there, seeing various starships being built, that young Rick developed a desire to join Starfleet and someday become a Captain. Against her father's wishes he joined Starfleet Academy rather than carry on the family tradition of shipbuilding. However, once in Starfleet, his short temper soon got him thrown out. Rather than return home in disgrace, he wandered for a time until he joined the Maquis. Once on Voyager, he proved to be loud mouthed and opinionated but extremely valuable in a firefight.",	//	MNT_HAZARD4_DESC8,

"TELSIA MURPHY, CREWMAN",																//	MNT_HAZARD5_DESC1,
"NAME : MURPHY, TELSIA CAITLIN",														//	MNT_HAZARD5_DESC2,
"RANK : CREWMAN (FORMER MAQUIS)",														//	MNT_HAZARD5_DESC3,
"BORN : FEBRUARY 29, 2351",																//	MNT_HAZARD5_DESC4,
"PLACE OF BIRTH : SURTUS PROVINCE, BELLATRIX V",										//	MNT_HAZARD5_DESC5,
"ASSIGNMENT : HAZARD TEAM SCOUT/SNIPER",												//	MNT_HAZARD5_DESC6,
"ACHIEVEMENTS : GARRIAT XII MARKSMANSHIP TOURNAMENT, 2369",								//	MNT_HAZARD5_DESC7,
"ORPHANED IN A CRIME-RIDDEN ORION SLUM ON BELLATRIX V, WHERE WOMEN WERE CONSIDERED LITTLE MORE THAN PROPERTY, TELSIA LEARNED TO FIGHT FOR WHAT SHE WANTED AT AN EARLY AGE. WITH THE AID OF A MENTOR, SHE WAS ABLE TO GAIN ENTRANCE TO STARFLEET ACADEMY BUT STRUGGLED UNDER THE RIGOROUS DISCIPLINE UNTIL SHE DROPPED OUT MIDWAY THROUGH HER LAST YEAR. A SHORT TIME LATER SHE JOINED THE MAQUIS, HER SPECIALITY BEING STEALTH INFILTRATION. THE SKILLS SHE LEARNED IN THE ACADEMY AND AS A YOUNG THIEF ON THE STREETS OF HER HOME CITY SERVED HER WELL. WHEN THE MAQUIS AND VOYAGER CREWS MERGED, SHE WAS ONE OF THE MORE BITTER VOICES OF DISCONTENT BUT HAS SINCE BECOME AN EXCELLENT AND PROFICIENT MEMBER OF THE CREW.",					//	MNT_HAZARD5_DESC8,

"CHELL, CREWMAN",																		//	MNT_HAZARD6_DESC1,
"NAME : CHELL",																			//	MNT_HAZARD6_DESC2,
"RANK : CREWMAN (FORMER MAQUIS)",														//	MNT_HAZARD6_DESC3,
"BORN : OCTOBER 5, 2346",																//	MNT_HAZARD6_DESC4,
"PLACE OF BIRTH : ASMIRA, PRIMARY CONTINANT, BOLIAX",									//	MNT_HAZARD6_DESC5,
"ASSIGNMENT : HAZARD TEAM, FIELD TECHNICIAN",											//	MNT_HAZARD6_DESC6,
"ACHIEVEMENTS : LISTED IN WHO'S WHO OF PLASMA TECHNOLOGY 2368",							//	MNT_HAZARD6_DESC7,
"Chell was born to an upper class Bolian family and while a teenager he came to resent their attitudes toward the poor and politics in general.  Relying on his natural aptitude for technical engineering, he applied to Starfleet Academy but was turned down.  After applying two more times, he was finally accepted, only to leave a year later, unhappy with the heavy workload. He soon joined the Maquis, drawn by their desire to go outside the rules and right what they viewed to be the Federation's wrongs. When he came aboard Voyager with his other Maquis crewmates, Chell was found to be cynical, lazy and a malcontent, unwilling to follow Starfleet protocol. He was one of the four Maquis deemed unruly and was personally trained by Tuvok. Since that time, he has taken his responsibilities seriously and gained Tuvok's respect. While he is adverse to battle and prone to complaining, he is a skilled technician and a valuable member of the Hazard Team.",	//	MNT_HAZARD6_DESC8,

"JULIET JUROT, CREWMAN",																//	MNT_HAZARD7_DESC1,
"NAME : JUROT, JULIET",																	//	MNT_HAZARD7_DESC2,
"RANK : CREWMAN",																		//	MNT_HAZARD7_DESC3,
"BORN : JUNE 3, 2352",																	//	MNT_HAZARD7_DESC4,
"PLACE OF BIRTH : QUANTANA BAY, BETAZED",												//	MNT_HAZARD7_DESC5,
"ASSIGNMENT : HAZARD TEAM, MEDIC",														//	MNT_HAZARD7_DESC6,
"ACHIEVEMENTS : VULCAN ACADEMY MEDICAL DEGREE, CUM LAUDE",								//	MNT_HAZARD7_DESC7,
"Juliet was interested in medicine from a young age, her compassion evident even as a child.  When it came time to choose a medical school, she chose the best and the most difficult - the Vulcan Science Academy, an odd choice for someone with the strong emotions of a Betazoid.  As a result of her training, she is a study in contrasts, acquiring something of the Vulcan demeanor and ideals. While she is compassionate and empathic like most Betazoids, she is often quiet, cold and logical.  She enlisted in Starfleet Academy's Medical Fast Track program designed to get medics into service quickly and was assigned to the U.S.S. Voyager.  When the Hazard Team was formed, she was chosen as field medic - the one member of the team designated to saving lives. Juliet firmly believes in the Vulcan axiom that the good of the many outweigh the good of the few and would freely sacrifice her life to save others.",	//	MNT_HAZARD7_DESC8,

"PERFECTO OVIEDO, CREWMAN",																//	MNT_HAZARD2_1_DESC1,
"NAME : OVIEDO, PERFECTO",																//	MNT_HAZARD2_1_DESC2,
"RANK : CREWMAN",																		//	MNT_HAZARD2_1_DESC3,
"BORN : SEPTEMBER 29, 2348",															//	MNT_HAZARD2_1_DESC4,
"PLACE OF BIRTH : IBIZA 7 DEEP SPACE COLONY",											//	MNT_HAZARD2_1_DESC5,
"ASSIGNMENT : HAZARD TEAM, QUARTERMASTER",												//	MNT_HAZARD2_1_DESC6,
"ACHIEVEMENTS : STARFLEET TACTICAL RESEARCH DIVISION SCHOLARSHIP",						//	MNT_HAZARD2_1_DESC7,
"Oviedo was born on one of the outermost planets of the Federation. This distance from Federation protection caused the small colony to constantly be on guard for attacks from any hostile aliens. Growing up in this environment, Oviedo was well aware of how weapons functioned at a young age. Through study and hard work, he managed to gain entrance into Starfleet where his aptitude for weapons technology became evident. Nicknamed 'Tech', he enjoys nothing more than to tear down a weapon and then reassemble it, piece by piece. He is constantly tweaking weapons, bringing their performance to just a little higher than intended. Oviedo was a natural choice as the Hazard Team's Quartermaster - if there's a piece of equipment needed he'll find it or build it. He keeps his armory fully stocked with weapons, ammo, field equipment and holodeck programs created by him to test weaponry.",//	MNT_HAZARD2_1_DESC8,

"KENN LATHROP, CREWMAN",																//	MNT_HAZARD2_2_DESC1,
"NAME : LATHROP, KENN",																	//	MNT_HAZARD2_2_DESC2,
"RANK : CREWMAN (FORMER MAQUIS)",														//	MNT_HAZARD2_2_DESC3,
"BORN : JANUARY 6, 2347",																//	MNT_HAZARD2_2_DESC4,
"PLACE OF BIRTH : BURLINGTON, CANADA, EARTH",											//	MNT_HAZARD2_2_DESC5,
"ASSIGNMENT : HAZARD TEAM, INTEL & OPERATIONS",											//	MNT_HAZARD2_2_DESC6,
"ACHIEVEMENTS : ",																		//	MNT_HAZARD2_2_DESC7,
"Lathrop's father was a hunter who taught his son to depend on himself and to live off the land. But his mother, a pianist with the Canadian Symphony, saw to it her son was aware of the more cultured things in life such as music and dance. A dissatisfied person, even in youth, Kenn was constantly in trouble at school for fighting with classmates and showing a total lack of respect for authority.  After a very brief career at Starfleet Academy, Lathrop joined the Maquis, finding their fight with the Cardassians a good way to release his aggressions. He is best friends with Kendrick Biessman from their days as Maquis. Lathrop is also an avid chess fan, but is routinely trounced by Juliet Jurot.",			//	MNT_HAZARD2_2_DESC8,

"THOMAS ODELL, CREWMAN",																//	MNT_HAZARD2_3_DESC1,
"NAME : ODELL, THOMAS FRANKLIN",														//	MNT_HAZARD2_3_DESC2,
"RANK : CREWMAN",																		//	MNT_HAZARD2_3_DESC3,
"BORN : MAY 3, 2350",																	//	MNT_HAZARD2_3_DESC4,
"PLACE OF BIRTH : OKLAHOMA CITY, OKLAHOMA, EARTH",										//	MNT_HAZARD2_3_DESC5,
"ASSIGNMENT : HAZARD TEAM, SCOUT",														//	MNT_HAZARD2_3_DESC6,
"ACHIEVEMENTS : DEGREE IN 20th CENTURY HISTORY",										//	MNT_HAZARD2_3_DESC7,
"Raised in Oklahoma, Odell spent his formative years hunting and farming. His mother is history professor and he acquired her love for antiques and history; he can talk at length about music and literature in the late 20th and early 21st centuries. His favorite subject of discussion is the ancient form of travel known as a 'motorcycle'. Back on Earth, he has a several restored vehicles which he would ride around his parent's farm - but given the difficulty of finding the gasoline needed to run the machines he rode very infrequently. He joined Starfleet to see the galaxy and has never regretted the decision. Rock steady even in the midst of a firefight, Odell is one of the first people the Hazard Team Beta Squad turns to in difficult situations.",	//	MNT_HAZARD2_3_DESC8,

"MICHAEL CSATLOS, CREWMAN",																//	MNT_HAZARD2_4_DESC1,
"NAME : CSATLOS, MICHAEL",																//	MNT_HAZARD2_4_DESC2,
"RANK : CREWMAN (FORMER MAQUIS)",														//	MNT_HAZARD2_4_DESC3,
"BORN : MARCH 17, 2351",																//	MNT_HAZARD2_4_DESC4,
"PLACE OF BIRTH : GALWAY, IRELAND, EARTH",												//	MNT_HAZARD2_4_DESC5,
"ASSIGNMENT : HAZARD TEAM, SCOUT",														//	MNT_HAZARD2_4_DESC6,
"ACHIEVEMENTS : FIRST PLACE, STARFLEET ACADEMY TRACK & FIELD 4K EVENT",					//	MNT_HAZARD2_4_DESC7,
"An eternal optimist, Csatlos is a bit of a riverboat gambler at heart. He enjoys taking chances, some would say he takes them too often. While this attitude gained much for him, it has had a high price - his career in Starfleet. After several training incidents which almost cost him and his teammate's lives he was asked to leave Starfleet. Csatlos left, hoping someday to return after he redeemed himself. But he then joined the Maquis and found it very rewarding, fighting for someone he believed to be the victims of injustice. He is also an avid runner; a track star in high school and later he won the Starfleet Academy Track and Field 4k event.",	//	MNT_HAZARD2_4_DESC8,

"GIOVANNI JAWORSKI, CREWMAN",															//	MNT_HAZARD2_5_DESC1,
"NAME : JAWORSKI, GIOVANNI MICHELE",													//	MNT_HAZARD2_5_DESC2,
"RANK : CREWMAN",																		//	MNT_HAZARD2_5_DESC3,
"BORN :APRIL 11, 2352",																	//	MNT_HAZARD2_5_DESC4,
"PLACE OF BIRTH : CORLEONE, SICILY, EARTH",												//	MNT_HAZARD2_5_DESC5,
"ASSIGNMENT : HAZARD TEAM, SCOUT",														//	MNT_HAZARD2_5_DESC6,
"ACHIEVEMENTS : THIRD PLACE, WORLD CUP SPELLING BEE 2364",								//	MNT_HAZARD2_5_DESC7,
"Gifted with a photographic memory, Jaworski did extremely well in school. When tested for intelligence, he was ranked in the top two percent of his age group. His parents were not surprised when he joined Starfleet but they were very disappointed when he went into security. They had thought with his intelligence he would have been a scientist. But he enjoyed the excitement and fast paced life of a security officer. When the Maquis joined the Voyager crew Jaworski was very outspoken on his dislike for the 'traitors' - this resulted in several altercations that ended in violence. Since that time, having worked closely with the former Maquis, he has grown to respect these people and considers them a valiant, courageous group.",	//	MNT_HAZARD2_5_DESC8,

"JEFFREY NELSON,CREWMAN",																//	MNT_HAZARD2_6_DESC1,
"NAME : NELSON, JEFFREY PAUL",															//	MNT_HAZARD2_6_DESC2,
"RANK : CREWMAN (FORMER MAQUIS)",														//	MNT_HAZARD2_6_DESC3,
"BORN : NOVEMBER 22, 2353",																//	MNT_HAZARD2_6_DESC4,
"PLACE OF BIRTH : CHINOOK, MONTANA, EARTH",												//	MNT_HAZARD2_6_DESC5,
"ASSIGNMENT : HAZARD TEAM, WEAPONS SPECIALIST",														//	MNT_HAZARD2_6_DESC6,
"ACHIEVEMENTS : WON PRESTIGOUS VAN GOGH SCHOLARSHIP, 2366",													//	MNT_HAZARD2_6_DESC7,
"The son of a rancher, he attended art school after showing great promise in painting and sculpture. But during his first semester he and several friends took spring break in on Luna; that one trip was enough to show Jeff the beauty of space - he fell in love with it instantly. He soon applied at Starfleet Academy and was accepted. Halfway through his first year Jeff quit, not wanting to wait for graduation to travel to distant worlds. He wandered for a time, working on freighters and doing odds jobs until he hooked up with the Maquis. He found their fight with Starfleet and the Cardassians to be a type of hopeless battle that he wanted to be a part of.  When the Maquis and Voyager crews combined Nelson began to see the value of Starfleet and now regrets his decision to leave the Academy.",																//	MNT_HAZARD2_6_DESC8,

"ELIZABETH LAIRD, CREWMAN",																//	MNT_HAZARD2_7_DESC1,
"NAME : LAIRD, ELIZABETH BEN",															//	MNT_HAZARD2_7_DESC2,
"RANK : CREWMAN",																		//	MNT_HAZARD2_7_DESC3,
"BORN : JUNE 16, 2350",																	//	MNT_HAZARD2_7_DESC4,
"PLACE OF BIRTH : HOBOKEN, NEW JERSEY, EARTH",											//	MNT_HAZARD2_7_DESC5,
"ASSIGNMENT : HAZARD TEAM, SCOUT",														//	MNT_HAZARD2_7_DESC6,
"ACHIEVEMENTS : ONE OF FEW HUMANS ALLOWED TRAINING ON Qo'noS",							//	MNT_HAZARD2_7_DESC7,
"Laird is a no-nonsense professional who was considered for the position of second in command of the Hazard Team. Her father is a martial arts instructor and her mother a gourmet chef; both of them extremely proud the day their daughter was accepted into Starfleet Academy. She is highly skilled in hand-to-hand combat, having attained a black belt in Aikido, Jujitsu, and Tae Kwan Do. Laird then studied for a year on Vulcan with the teachers of physical combat. She then went to the Klingon home world of Qo'noS to study weapons. While on Qo'noS she was forced to kill a weapons master in battle and was presented with their highest medal of honor. She is also skilled with bow and arrow, phaser rifle, and the bat'telh. She was on the martial arts team representing Earth at the Federation Olympics.",	//	MNT_HAZARD2_7_DESC8,

// Video Setup Menu
"HIGH QUALITY",																//	MNT_VIDEO_HIGH_QUALITY
"NORMAL",																	//	MNT_VIDEO_NORMAL
"FAST",																		//	MNT_VIDEO_FAST
"FASTEST",																	//	MNT_VIDEO_FASTEST
"CUSTOM",																	//	MNT_VIDEO_CUSTOM
"DEFAULT",																	//	MNT_VIDEO_DRIVER_DEFAULT
"VOODOO",																	//	MNT_VIDEO_DRIVER_VOODOO
"OFF",																		//	MNT_OFF
"ON",																		//	MNT_ON
"320X240",																	//	MNT_320X200
"400X300",																	//	MNT_400X300
"512X384",																	//	MNT_512X384
"640X480",																	//	MNT_640X480
"800X600",																	//	MNT_800X600
"960X720",																	//	MNT_960X720
"1024x768",																	//	MNT_1024X768
"1152x864",																	//	MNT_1152X864
"1280x960",																	//	MNT_1280X960
"1600x1200",																//	MNT_1600X1200
"2048x1536",																//	MNT_2048X1536
"856x480 Wide Screen",														//	MNT_856x480WIDE
"DEFAULT",																	//	MNT_DEFAULT
"16-BIT",																	//	MNT_16BIT
"32-BIT",																	//	MNT_32BIT
"LIGHTMAP",																	//	MNT_LIGHTMAP
"VERTEX",																	//	MNT_VERTEX
"LOW",																		//	MNT_LOW
"MEDIUM",																	//	MNT_MEDIUM
"HIGH",																		//	MNT_HIGH
"VERY HIGH",																//	MNT_VERY_HIGH
"BILINEAR",																	//	MNT_BILINEAR
"TRILINEAR",																//	MNT_TRILINEAR

// Video Setup Menu
"VIDEO SETUP",																//	MNT_VIDEOSETUP

// Video Driver Menu
"VIDEO DRIVER",																//	MNT_VIDEODRIVER
"VIDEO DRIVERS",															//	MNT_VIDEODRIVERS
"VENDOR:",																	//	MNT_VENDOR
"VERSION:",																	//	MNT_VERSION
"RENDERER:",																//	MNT_RENDERER
"PIXELFORMAT:",																//	MNT_PIXELFORMAT
"EXTENSIONS:",																//	MNT_EXTENSIONS

// Main Menu
"ALPHA",																	//	MNT_ALPHA
"STELLAR CARTOGRAPHY",														//	MNT_STELLAR_CARTOGRAPHY,
"THE UNITED FEDERATION OF PLANETS, FOUNDED IN 2161. CAPITAL PLANET:EARTH.",	//	MNT_FEDERATION_TXT
"SIGNED TRUCE WITH FEDERATION IN 2367. CAPITAL PLANET:CARDASSIA PRIME.",	//	MNT_CARDASSIA_TXT
"CULTURE WHICH RUTHLESSLY EMBRACES CAPITALISM. CAPITAL PLANET:FERENGINAR.",	//	MNT_FERENGI_TXT
"AN OFFSHOOT OF THE VULCAN CULTURE. CAPITAL PLANETS:ROMULUS & REMUS.",		//	MNT_ROMULAN_TXT
"HAD FIRST CONTACT WITH FEDERATION IN 2218. CAPITAL PLANET:Q'ONOS.",		//	MNT_KLINGON_TXT
"POWERFUL ALLIANCE OF GAMMA QUADRANT PLANETARY GROUPS.",					//	MNT_DOMINION_TXT
"NCC-74656, INTREPID CLASS STARSHIP, CURRENTLY RETURNING FROM DELTA QUADRANT.",	//	MNT_VOYAGER_TXT
"CYBORG RACE, FIRST INTRODUCED TO THE FEDERATION BY ENTITY KNOWN AS 'Q'",	//	MNT_BORG_TXT
"ARTIFICIAL 70,000 LIGHT YEAR PASSAGEWAY BETWEEN BAJOR AND GAMMA QUADRANT",	//	MNT_WORMHOLE_TXT
"CENTER OF THE MILKY WAY GALAXY, RUMORED LOCATION OF SHA KA REE.",			//	MNT_GALACTICCORE_TXT
"ONE QUARTER OF MILKY WAY GALAXY, CONTAINS THE UNITED FEDERATION OF PLANETS.",	//	MNT_ALPHAQUAD_TXT
"ONE QUARTER OF MILKY WAY GALAXY, CONTAINS THE KLINGON AND ROMULAN EMPIRES.",	//	MNT_BETAQUAD_TXT
"ONE QUARTER OF MILKY WAY GALAXY, UNEXPLORED, BELIEVED HOME OF THE BORG.",	//	MNT_DELTAQUAD_TXT
"ONE QUARTER OF MILKY WAY GALAXY, CURRENTLY EXPLORED BY USE OF BAJORAN WORMHOLE.",	// MNT_GAMMAQUAD_TXT

// Ingame Main Menu
"ELITE FORCE : INGAME MAIN MENU",											//	MNT_INGAMEMENU_TITLE
"HAZARD SUIT SPECS",														//	MNT_HAZARDSUIT_SPECS,
"HAZARD SUIT",																//	MNT_HAZARDSUIT
"FRONT",																	//	MNT_FRONT
"BACK",																		//	MNT_BACK
"POWER CONVERTER",															//	MNT_POWER_CONVERTER_LABEL
"COMM BADGE",																//	MNT_COMMBADGE_LABEL,
"LOGISTICS & DIRECTIONAL",													//	MNT_LOGISTICS_LABEL,
"ENERGY PACK",																//	MNT_ENERGY_PACK_LABEL
"MULTI-PHASIC WAVE GENERATORS",												//	MNT_WAVEGENERATOR_LABEL
"PASSIVE/ACTIVE SCANNERS",													//	MNT_SCANNERS_LABEL
"EQUIPMENT POUCH",															//	MNT_POUCHES_LABEL
"TRANSPORTER BUFFER",														//	MNT_BUFFER_LABEL


"POWER CONVERTER : used to convert alien energy into a source of power for Federation weapons",						//	MNT_POWERCONVERTER_DESC1,
"COMM BADGE : for use in communicating between team members and Voyager",											//	MNT_COMMBADGE_DESC1,
"DIRECTION/LOGISTICS : supplies data to tactical display, giving heading, location, and location of team members",	//	MNT_LOGISTICS_DESC1,
"ENERGY PACK : stores power for Federation weapons",																//	MNT_ENERGY_PACK_DESC1,
"MULTI_PHASIC WAVE GENERATOR : used to disrupt lock ons of unfriendly transporter beams.",							//	MNT_WAVEGENERATOR_DESC1,
"ACTIVE/PASSIVE SCANNERS : gives information of the surrounding environment.",										//	MNT_SCANNERS_DESC1,
"EQUIPMENT POUCHES : for holding various supplies",																	//	MNT_POUCHES_DESC1,
"TRANSPORTER BUFFER : stores equipment in a molcularized state which can be rematerialized instantly when needed.",	//	MNT_BUFFER_DESC1,

// Quit Menu
"QUIT ELITE FORCE?",														//	MNT_QUIT_ELITE_FORCE

"BRIDGE: Control center of the ship. Conn (Flight Controller) is both helm and navigation, Ops (Operations) allocates resources such as power,sensor and computer time, Tactical deals with weaponry, defense, and internal security.",		//	MNT_BRIDGE_DESC1
"FORWARD PHASERS: phasers are the primary offensive weapon of Voyager. They are a particle/beam projector that can only be used at sub-light speeds but have an effective range of 300,000 kilometers.",									//	MNT_PHASER_STRIP1_DESC1
"FORWARD PHOTON TORPEDO LAUNCHERS: Photon torpedoes carry a small amount of matter and antimatter bound together in a magnetic bottle. Torpedoes are launched at warp speed making them ideal for use during faster than light travel.",	//	MNT_TORPEDOS_DESC1
"VENTRAL PHASERS: the phaser strip located along the bottom section of Voyager. Phaser is short for PHASED ENERGY RECTIFICATION.",																											//	MNT_VENTRAL_DESC1
"MID-HULL PHASER STRIP: located on both sides of Voyager, these have a firing arc of 180 degrees along either side of the ship.",																											//	MNT_MIDHULL_DESC1
"BUSSARD COLLECTORS : Electromagnetic devices designed to collect hydrogen gas for use by ships fusion reactors. Concept initially proposed by physicist Robert W. Bussard in 1960.",														//	MNT_BUSSARD_DESC1
"WARP NACELLES : Large outboard structures which house the warp-drive engines. The nacelles raise when the engines are not in use. The articulated design allows use of warp speed without damaging the fabric of space.",					//	MNT_NACELLES_DESC1
"RCS (MANUVERING) THRUSTERS: propulsion units designed to move Voyager at very low speeds. Useful for navigating the ship in and around small areas such as space docks.",																	//	MNT_THRUSTERS_DESC1
"VOYAGER : STARSHIP NCC - 74656: 344 meters long, 1.5 million metric tons with a basic hull structure of tritanium/duranium.Built at Earth Station McKinley and launched on Stardate 48038.5. Commanded by Captain Kathryn Janeway.",		//	MNT_VOYAGER_DESC1

"SHIP SYSTEMS",																//	MNT_SHIP_SYSTEMS

// Credits Screen
"CREDITS",																	//	MNT_CREDITS

// Game Option Menu
"GAME OPTIONS SETUP",														//	MNT_GAMEOPTION_LABEL

// Setup / Sound
"SOUND SETUP",																//	MNT_SOUND_SETUP

// Setup / Controls / Weapons
"WEAPON KEYS SETUP",														//	MNT_WEAPONKEY_SETUP

// Setup / Controls / Attack/Look
"ATTACK/LOOK KEYS SETUP",													//	MNT_ATTACKLOOKKEY_SETUP

// Setup / Controls / Movement
"MOVEMENT KEYS SETUP",														//	MNT_MOVEMENTKEYS_SETUP

// Setup / Controls / Commands
"COMMAND KEYS SETUP",														//	MNT_COMMANDKEYS_SETUP

// Setup / Controls / Mouse/JoyStick
"MOUSE/JOYSTICK SETUP",														//	MNT_MOUSEJOYSTICK_SETUP
"MOUSE",																	//	MNT_MOUSE,
"JOYSTICK",																	//	MNT_JOYSTICK,

// Setup / Controls / Mouse/ Other Options
"OTHER OPTIONS SETUP",														//	MNT_OTHEROPTIONS_SETUP,

// New Game Screen
"WARP CORE STARTUP",														// MNT_WARP_CORE_STARTUP,
"GAME DIFFICULTY",															// MNT_GAME_DIFFICULTY,
"GENDER",																	//	MNT_GENDER,

// Load Game Screen
"LOAD GAME",																//	MNT_LOADGAME,
"CURRENT SAVE GAMES",														//	MNT_CURRENTSAVEGAMES,

// Save Game Screen
"SAVE GAME",																//	MNT_SAVEGAME,
"DESCRIPTION",																//	MNT_DESCRIPTION
"CURRRENT SAVE GAMES",														//	MNT_CURRENTSAVEGAME

"Transporting to",															// MNT_TRANSPORTING
"TARGETING SCANNERS",														// MNT_TARGETINGSCANNERS
"CONFIRMING BEAM-IN POINT",													// MNT_CONFIRMINGBEAMIN
"LOCKING POSITION",															//MNT_LOCKINGPOSTION
"SCANNING AREA",															//	MNT_SCANNINGAREA
"INITIATING TRANSPORT",														//	MNT_INITIATINGTRANSPORT

"NONE",																		//	MNT_CROSSHAIR_NONE

"STAR TREK : VOYAGER - ELITE FORCE",										// MNT_STARTREK
"BY RAVEN SOFTWARE",														// MNT_BYRAVENSOFTWARE	
"VISIT THE WEB SITE",														// MNT_VISITWEBSITE

"Restarting the sound system. This could take up to a minute, so please be patient.", //	MNT_RESTARTINGSOUND
"Say:",																		//	MNT_SAY
"AUTO SAVE",																//	MNT_AUTOSAVE

"Please enter your CD Key",													//	MNT_ENTER_CDKEY
"The CD Key is valid, thank you.",											//	MNT_VALID_CDKEY	
"The CD Key is not valid. Try again."										//	MNT_CDKEY_INVALID

};
*/


// BUTTON TEXT and DESCRIPTIONS

char *menu_button_text[MBT_MAX][2]; 
/*
char *menu_button_text[MBT_MAX][2] = 
{
"",							NULL,											//	MBT_NONE
"NEW GAME",					"START A NEW GAME",								//	MBT_NEWGAME
"LOAD GAME",				"LOAD A PREVIOUSLY SAVED GAME",					//	MBT_LOADGAME
"SETUP",					"CREATE KEY BINDINGS",							//	MBT_SETUP
"EXPLORE VOYAGER",			"WALK THROUGH THE LEVELS OF VOYAGER",			//	MBT_EXPLOREVGER
"CREDITS",					"MEET THE CREWMEN WHO CREATED ELITE FORCE.",	//	MBT_CREDITS
"QUIT",						"LEAVE GAME",									//	MBT_QUIT
"VOYAGER CREW",				"VIEW CREW BIOGRAPHIES.",						// MBT_VOYAGERCREW

// Ingame Main Menu
"SAVE GAME",				"SAVE CURRENT GAME TO DISK",					//	MBT_SAVEGAME,
"LOAD GAME",				"LOAD GAME FROM DISK",							//	MBT_IMLOADGAME,
"POWER CONVERTER",			"DATA ON POWER CONVERTER",						//	MBT_POWERCONV
"COMM BADGE",				"DATA ON COMM BADGE",							//	MBT_COMMBADGE
"DIRECTION/LOGISTICS",		"DATA ON DIRECTIONAL/LOGISTICS CIRCUITRY",		//	MBT_LOGISTICS
"ENERGY PACK",				"DATA ON ENERGY PACK",							//	MBT_ENERGY_PACK
"MULTI-PHASIC WAVE GENERATORS","DATA ON WAVE GENERATORS",					//	MBT_WAVEGENERATOR
"PASSIVE/ACTIVE SCANNERS",	"DATA ON SCANNERS",								//	MBT_SCANNERS
"EQUIPMENT POUCH",			"DATA ON EQUIPMENT POUCHES",					//	MBT_POUCHES
"TRANSPORTER BUFFER",		"DATA ON WEAPON BUFFERS",						//	MBT_BUFFER
"QUIT GAME",				"QUIT CURRENT GAME",							//	MBT_QUITGAME,
"EXIT PROGRAM",				"QUIT ELITE FORCE PROGRAM",						//	MBT_EXITPROG

"INGAME MENU",				"RETURN TO INGAME MENU",						//	MBT_INGAMEMENU

// Loadgame Menu
"LOAD",						"LOAD CURRENT GAME FROM DISK",					//	MBT_LOADGAMEDATA,

// Savegame Menu
"SAVE",						"SAVE CURRENT GAME TO DISK",					//	MBT_SAVEGAMEDATA,
"DELETE",					"DELETE SAVE GAME FROM DISK",					//	MBT_DELETEGAMEDATA,
NULL,						"SAVE GAME DESCRIPTION",						//	MBT_SAVEGAMELINE

"FEDERATION",				"DATA ON THE FEDERATION",						// MBT_FEDERATION_LABEL
"CARDASSIA",				"DATA ON THE CARDASSIAN EMPIRE",				// MBT_CARDASSIA_LABEL
"FERENGI ALLIANCE",			"DATA ON THE FERENGI ALLIANCE",					// MBT_FERENGI_LABEL
"ROMULAN EMPIRE",			"DATA ON THE ROMULAN EMPIRE",					// MBT_ROMULAN_LABEL
"KLINGON EMPIRE",			"DATA ON THE KLINGON EMPIRE",					// MBT_KLINGON_LABEL
"DOMINION",					"DATA ON THE DOMINION",							// MBT_DOMINION_LABEL
"VOYAGER STARTING POINT",	"DATA ON VOYAGER",								// MBT_VOYAGER_LABEL1
"75,000 LIGHT YEARS",		NULL,											// MBT_VOYAGER_LABEL2
"BORG",						"DATA ON THE BORG",								// MBT_BORG_LABEL1
"SPACE",					NULL,											//	MBT_BORG_LABEL2
"IDRAN END OF",				"DATA ON BAJORAN WORMHOLE",						// MBT_WORMHOLE_LABEL1
"BAJORAN WORMHOLE",			NULL,											//	MBT_WORMHOLE_LABEL2

"GALACTIC CORE",			"DATA ON THE GALACTIC CORE",					// MBT_GALACTICCORE_LABEL1

"ALPHA",					"DATA ON THE ALPHA QUADRANT",					// MBT_ALPHA
"BETA",						"DATA ON THE BETA QUADRANT",					// MBT_BETA
"DELTA",					"DATA ON THE DELTA QUADRANT",					// MBT_DELTA
"GAMMA",					"DATA ON THE GAMMA QUADRANT",					// MBT_GAMMA
"QUADRANT",					NULL,											// MBT_QUADRANT

"FORWARD PHASER",			"DATA ON FORWARD PHASERS",						// MBT_V_PHASER_LABEL
"FORWARD",					"DATA ON FORWARD TORPEDOS",						// MBT_V_TORPEDOS_LABEL
"PHOTON TORPEDO",			NULL,											// MBT_V_TORPEDOS_LABEL2
"LAUNCHERS",				NULL,											// MBT_V_TORPEDOS_LABEL3

"VENTRAL PHASER STRIP",		"DATA ON VENTRAL PHASER",						// MBT_V_VENTRAL_LABEL
"MID-HULL",					"DATA ON MID-HULL PHASER",						// MBT_V_MIDHULL_LABEL
"PHASER STRIP",				NULL,											// MBT_V_MIDHULL_LABEL2
"BUSSARD COLLECTORS",		"DATA ON BUSSARD COLLECTORS",					// MBT_V_BUSSARD_LABEL
"WARP NACELLES IN LOWERED POSITION","DATA ON WARP NACELLES",				// MBT_V_NACELLES_LABEL
"RCS (MANUVERING) THRUSTERS",	"DATA ON THRUSTERS",						// MBT_V_THRUSTERS_LABEL
"U.S.S. VOYAGER",				"DATA ON VOYAGER",								// MBT_V_VOYAGER_LABEL
"BRIDGE",					"DATA ON THE BRIDGE",							// MBT_V_BRIDGE_LABEL

// New Game 
"EASY",						"FOR CADETS",									//	MBT_EASY
"MEDIUM",					"FOR SEASONED CREWMEN",							//	MBT_MEDIUM
"HARD",						"ONLY FOR THE MOST EXPERIENCED VETERANS",		//	MBT_HARD
"MALE",						"BE ALEXANDER MUNRO",							//	MBT_MALE
"FEMALE",					"BE ALEXANDREA MUNRO",							//	MBT_FEMALE
"ENGAGE",					"START GAME",									//	MBT_ENGAGE
"MAIN MENU",				"RETURN TO MAIN MENU.",							//	MBT_MAINMENU
"VIDEO",					"VIDEO SETTINGS",								//	MBT_VIDEOSETTINGS,
"CONTROLS",					"SET KEY BINDINGS AND MOUSE CONFIGURATION.",	//	MBT_CONTROLS,
"SOUND",					"SOUND SETTINGS",								//	MBT_SOUNDSETTINGS
"GAME OPTIONS",				NULL,											//	MBT_GAMEOPTIONS
"CD KEY",					NULL,											//	MBT_CDKEY
"FONTS",					"VIEW CURRENT FONTS AVAILABLE",					//	MBT_FONTS
"SMALL",					"VIEW SMALL FONT",								//	MBT_FONTSSMALL,
"MEDIUM",					"VEIEW MEDIUM FONT",							//	MBT_FONTSMEDIUM,
"LARGE",					"VIEW LARGE FONT",								//	MBT_FONTSLARGE,
"WEAPONS",					"CONFIGURATE WEAPON KEYS",						//	MBT_WEAPONKEYS
"COMMANDS",					"COMFIGURATE COMMAND KEYS",						//	MBT_COMMANDKEYS
"ATTACK/LOOK",				"CONFIGURATE ATTACK/LOOK KEYS",					//	MBT_LOOKKEYS
"MOVEMENT",					"CONFIGURATE MOVEMENT KEYS",					//	MBT_MOVEMENTKEYS
"MOUSE/JOYSTICK",			"SET MOUSE AND JOYSTICK OPTIONS",				//	MBT_MOUSECONFIG
"OTHER OPTIONS",			"ADDITIONAL OPTIONS",							//	MBT_OTHEROPTIONS
"LOAD CONFIG",				"LOAD A CONFIG FROM DISK",						//	MBT_LOADCONFIG
"SAVE CONFIG",				"SAVE CURRENT CONFIG",							//	MBT_SAVECONFIG
"YES",						"LEAVE GAME",									//	MBT_QUITGAME_YES,
"NO",						"RETURN TO MAIN MENU.",							//	MBT_QUITGAME_NO
"VIDEO OPTIONS",			"CHOOSE GROUP VIDEO MODE SETTINGS",				//	MBT_VIDEOOPTIONS
"GL DRIVER",				"SET GL DRIVER (VOODOO or DEFAULT)",			//	MBT_VIDEODRIVER
"GL EXTENSIONS",			"ALLOW GL EXTENSIONS (Y/N)",					//	MBT_VIDEOGLEXTENTIONS	
"VIDEO MODE",				"CURRENT VIDEO MODE",							//	MBT_VIDEOMODE
"COLOR DEPTH",				"16 OR 32 BITS",								//	MBT_VIDEOCOLORDEPTH
"FULL SCREEN",				"RUN IN FULL SCREEN OR WINDOWED MODE",			//	MBT_VIDEOFULLSCREEN
"LIGHTING",					"LIGHTMAP OR VERTEX",							//	MBT_VIDEOLIGHTING
"GEOMETRIC DETAIL",			"LEVEL OF GEOMETRIC DETAIL",					//	MBT_VIDEOGEOMETRY
"TEXTURE DETAIL",			"LEVEL OF TEXTURE DETAIL",						//	MBT_VIDEOTEXTUREDETAIL
"TEXTURE QUALITY",			"LEVEL OF TEXTURE BITS",						//	MBT_VIDEOTEXTUREBITS
"TEXTURE FILTER",			"BILINEAR OR TRILINEAR",						//	MBT_VIDEOTEXTUREFILTER
"APPLY CHANGES",			"APPLY ANY CHANGES MADE NOW",					//	MBT_VIDEOAPPLYCHANGES
"VIDEO DATA",				"UPDATE VIDEO INFORMATION",						//	MBT_VIDEODATA,
"VIDEO DATA 2",				"UPDATE ADDITIONAL VIDEO INFORMATION",			//	MBT_VIDEODATA2,
"VIDEO DRIVERS",			"VIDEO DRIVER INFORMATION",						//	MBT_VIDEODRIVERS,

//	Video 2 Data
"BRIGHTNESS",				"SET LEVEL OF BRIGHTNESS IN GAME",				//	MBT_BRIGHTNESS,
"INTENSITY",				"SET INTENSITY",								//	MBT_INTENSITY,
"SCREEN SIZE",				"SET SIZE OF PLAYABLE SCREEN",					//	MBT_SCREENSIZE,

// Voyager Crew
"SENIOR STAFF",				"VIEW BIOGRAPHIES OF SENIOR STAFF",				//	MBT_SENIORDATA,
"HAZARD TEAM",				"VIEW BIOGRAPHIES OF HAZARD TEAM",				//	MBT_HAZARDDATA,
"ALPHA SQUAD",				"VIEW ALHPA SQUAD",								//	MBT_ALPHASQUAD,
"BETA SQUAD",				"VIEW BETA SQUAD",								//	MBT_BETASQUAD,
"CREWMEN",					"VIEW BIOGRAPHIES OF CREWMEN",					//	MBT_CREWMENDATA,
"CAPTAIN JANEWAY",			"VIEW BIOGRAPHY OF CAPTAIN KATHRYN JANEWAY.",	//	MBT_JANEWAY
"CMDR. CHAKOTAY",			"VIEW BIOGRAPHY OF CHAKOTAY.",					//	MBT_CHAKOTAY
"LT. CMRD. TUVOK",			"VIEW BIOGRAPHY OF TUVOK.",						//	MBT_TUVOK
"LT. TORRES",				"VIEW BIOGRAPHY OF TUORRES.",					//	MBT_TORRES
"LT. PARIS",				"VIEW BIOGRAPHY OF TOM PARIS.",					//	MBT_PARIS
"ENSIGN KIM",				"VIEW BIOGRAPHY OF HARRY KIM.",					//	MBT_KIM
"DOCTOR",					"VIEW BIOGRAPHY OF HOLOGRAPHIC DOCTOR.",		//	MBT_DOCTOR
"SEVEN OF NINE",			"VIEW BIOGRAPHY OF SEVEN OF NINE.",				//	MBT_SEVEN
"NEELIX",					"VIEW BIOGRAPHY OF NEELIX.",					//	MBT_NEELIX

"LT. FOSTER",				"VIEW BIOGRAPHY LT. LES FOSTER.",				//	MBT_FOSTER
"ENSIGN MUNRO",				"VIEW BIOGRAPHY ALEXANDER MUNRO.",				//	MBT_MUNRO
"CREWMAN CHANG",			"VIEW BIOGRAPHY AUSTIN CHANG.",					//	MBT_CHANG
"CREWMAN BIESSMAN",			"VIEW BIOGRAPHY RICK BIESSMAN.",				//	MBT_BIESSMAN
"CREWMAN MURPHY",			"VIEW BIOGRAPHY TELSIA MURPHY.",				//	MBT_MURPHY
"CREWMAN CHELL",			"VIEW BIOGRAPHY CHELL.",						//	MBT_CHELL
"CREWMAN JUROT",			"VIEW BIOGRAPHY JUROT.",						//	MBT_JUROT

"CREWMAN CUERVO",			"VIEW BIOGRAPHY JESUS CUERVO.",					//	MBT_CUERVO
"CREWMAN LATHROP",			"VIEW BIOGRAPHY KENN LATHROP.",					//	MBT_LATHROP
"CREWMAN ODELL",			"VIEW BIOGRAPHY THOMAS ODELL.",					//	MBT_ODELL
"CREWMAN CSATLOS",			"VIEW BIOGRAPHY MICHAEL CSATLOS.",				//	MBT_CSATLOS
"CREWMAN JAWORSKI",			"VIEW BIOGRAPHY GIOVANNI JAWORSKI.",			//	MBT_JAWORSKI
"CREWMAN NELSON",			"VIEW BIOGRAPHY JEFFREY NELSON.",				//	MBT_NELSON
"CREWMAN LAIRD",			"VIEW BIOGRAPHY ELIZABETH LAIRD.",				//	MBT_LAIRD

// Setup/ Controls/ Mouse Config
"FREE LOOK",				"FREE LOOK Y/N",								//	MBT_MOUSEFREELOOK
"MOUSE SPEED",				"SPEED MOUSE MOVES ACROSS SCREEN.",				//	MBT_MOUSESPEED
"INVERT MOUSE",				"WHEN ON, PULLING MOUSE BACK LIFTS VIEW IN GAME.",	//	MBT_MOUSEINVERT
"SMOOTH MOUSE",				"SMOOTH MOUSE (Y/N)",							//	MBT_MOUSESMOOTH
"ENABLE JOYSTICK",			"ENABLE JOYSTICK (Y/N)",						//	MBT_JOYSTICKENABLE
"JOYSTICK THRESHOLD",		"THRESHOLD OF JOYSTICK",						//	MBT_JOYSTICKTHRESHOLD
"FORCE FEEDBACK",			"JOYSTICK USES FORCEFEEDBACK",					//	MBT_FORCEFEEDBACK

// Setup/ Controls / Other Options
"ALWAYS RUN",				"PLAYER ALWAYS IN RUN MODE.",					//	MBT_ALWAYSRUN,
"LOOK SPRING",				"LOOK SPRING",									//	MBT_LOOKSPRING,
"KEY TURN SPEED",			"KEY TURN SPEED",								//	MBT_KEYTURNSPEED,
"DISABLE ALT-TAB",			"TURN OFF WINDOWS ALT-TAB FEATURE",				//	MBT_DISABLE_ALTTAB
"CAPTIONING",				"TURN OFF CAPTIONING DURING GAME",				//	MBT_CAPTIONING
"CONSOLE KEY",				"TO ACTIVIATE/DE-ACTIVATE CONSOLE",				//	MBT_CONSOLE
"AUTOSWITCH WEAPONS",		"AUTOSWITCH WEAPONS WHEN OUT OF AMMO",			//	MBT_AUTOSWITCHWEAPONS

// Setup / Sound
"EFFECTS VOLUME",			"VOLUME FOR GAME EFFECTS.",						// MBT_EFFECTSVOLUME
"MUSIC VOLUME",				"VOLUME FOR GAME MUSIC.",						// MBT_MUSICVOLUME
"VOICE VOLUME",				"VOLUME FOR VOICES IN GAME.",					// MBT_VOICEVOLUME
"SOUND QUALITY",			"SOUND QUALITY.",								// MBT_SOUNDQUALITY
"A3D",						"A3D SOUND ON/OFF.",							// MBT_A3D


"RESUME GAME",				"RETURN TO CURRENT GAME",						// MBT_RETURNTOGAME
"WAITING FOR KEY OR ESCAPE TO CANCEL",	NULL,								// MBT_WAITFORKEY

"ON",						NULL,											// MBT_ON
"OFF",						NULL,											// MBT_OFF

"WALL",						"DAMAGE MARKS ON WALLS",						// MBT_WALLMARKS1
"MARKS",					NULL,											// MBT_WALLMARKS2
"DYNAMIC",					"TURN ON/OFF DYNAMIC LIGHTS",					// MBT_DYNAMICLIGHTS1
"LIGHTS",					NULL,											// MBT_DYNAMICLIGHTS2
"LIGHT",					"TURN ON/OFF LIGHT FLARES",						// MBT_LIGHTFLARES1
"FLARES",					NULL,											// MBT_LIGHTFLARES2
"IDENTIFY",					"TURN ON/OFF TARGET IDENTIFIER",				// MBT_IDENTIFYTARGET1
"TARGET",					NULL,											// MBT_IDENTIFYTARGET2
"SYNC EVERY",				"TURN ON/OFF FRAME SYNC",						// MBT_SYNCEVERYFRAME1
"FRAME",					NULL,											// MBT_SYNCEVERYFRAME2
"CROSSHAIR",				"SET GAME CROSSHAIR",							// MBT_CROSSHAIR

"APPLY",					"APPLY ANY CHANGES MADE NOW",					//	MBT_APPLYCHANGES1
"CHANGES",					NULL,											//	MBT_APPLYCHANGES2
};
*/

sfxHandle_t	menu_in_sound;
sfxHandle_t	menu_move_sound;
sfxHandle_t	menu_out_sound;
sfxHandle_t	menu_buzz_sound;
sfxHandle_t	menu_null_sound = 0;

vec4_t	menu_grayed_color	 = {0.5, 0.5, 0.5, 1.0};
vec4_t	menu_text_color	     = {1.0, 1.0, 1.0, 1.0};
vec4_t	menu_dark_color	     = {0.0, 1.0, 0.0, 1.0};
vec4_t	menu_highlight_color = {1.0, 1.0, 0.0, 1.0};
vec4_t	menu_lowlight_color  = {0.8f, 0.8f, 0.0, 1.0};
vec4_t	menu_dim_color       = {0.0, 0.0, 0.0, 0.75};

vec4_t color_black	        = {0.00, 0.00, 0.00, 1.00};
vec4_t color_white	        = {1.00, 1.00, 1.00, 1.00};
vec4_t color_yellow	        = {1.00, 1.00, 0.00, 1.00};
vec4_t color_blue	        = {0.00, 0.00, 1.00, 1.00};
vec4_t color_lightOrange    = {1.00, 0.68f, 0.00, 1.00 };
vec4_t color_orange	        = {1.00, 0.43f, 0.00, 1.00};
vec4_t color_red		    = {1.00, 0.00, 0.00, 1.00};
vec4_t color_dim		    = {0.00, 0.00, 0.00, 0.25};

// current color scheme
vec4_t pulse_color          = {1.00, 1.00, 1.00, 1.00};
vec4_t text_color_disabled  = {0.50, 0.50, 0.50, 1.00};	// light gray
vec4_t text_color_normal	= {1.00, 0.43f, 0.00, 1.00};	// light orange
vec4_t text_color_highlight = {1.00, 1.00, 0.00, 1.00};	// bright yellow
vec4_t listbar_color        = {1.00, 0.43f, 0.00, 0.30f};	// transluscent orange
vec4_t text_color_status    = {1.00, 1.00, 1.00, 1.00};	// bright white	

// action widget
static void	Action_Init( menuaction_s *a );
static void	Action_Draw( menuaction_s *a );
static void Action_Focus( menuaction_s *a );

// listbox widget
static void	ListBox_Init( menulist_s *l );
static void	ListBox_Focus( menulist_s *l );
static sfxHandle_t  ListBox_Key( menulist_s *l, int key );
static void	ListBox_Draw( menulist_s *l );

// radio button widget
static void	RadioButton_Init( menuradiobutton_s *rb );
static void	RadioButton_Draw( menuradiobutton_s *rb );
static sfxHandle_t RadioButton_Key( menuradiobutton_s *rb, int key );


// separator widget
static void	Separator_Draw( menuseparator_s *s );

// slider widget
static void Slider_Init( menuslider_s *s );
static sfxHandle_t Slider_Key( menuslider_s *s, int key );
static void	Slider_Draw( menuslider_s *s );

// spin control widget
static void	SpinControl_Init( menulist_s *s );
static void	SpinControl_Draw( menulist_s *s );
static sfxHandle_t SpinControl_Key( menulist_s *l, int key );

// bitmap widget
static void Bitmap_Init( menubitmap_s *b );
static void Bitmap_Focus( menubitmap_s *b );
static void Bitmap_Draw( menuframework_s *menu,int index );

// text widget
static void Text_Init( menutext_s *b );
static void Text_Draw( menuframework_s *menu,menutext_s *b );

// scrolllist widget
static void	ScrollList_Init( menulist_s *l );
static sfxHandle_t ScrollList_Key( menulist_s *l, int key );
static void	ScrollList_Draw( menulist_s *l );


/*
===============
Text_Init
===============
*/
static void Text_Init( menutext_s *t )
{
	int	x;
	int	y;
	int	w,w2,w3;
	int	h,lines;
	char	buff[512];	
	char	buff2[512];	
	char	buff3[512];	

	buff[0] = '\0';
	buff2[0] = '\0';
	buff3[0] = '\0';

	if (t->focusX)
	{
		x = t->focusX;
		y = t->focusY;
		w = t->focusWidth;
		h =	t->focusHeight;
	}
	else
	{
		// Button text (text can be clicked on)
		if (t->buttontextEnum)
		{
			w = w2 = w3 = 0;

			if (menu_button_text[t->buttontextEnum][0])
			{
				strcpy(buff,menu_button_text[t->buttontextEnum][0]);
				w = UI_ProportionalStringWidth( buff,t->style);
			}

			if (t->buttontextEnum2)
			{
				if (menu_button_text[t->buttontextEnum2][0])
				{
					strcpy(buff2,menu_button_text[t->buttontextEnum2][0]);
					w2 = UI_ProportionalStringWidth( buff2,t->style);
				}
			}

			if (t->buttontextEnum3)
			{
				if (menu_button_text[t->buttontextEnum3][0])
				{
					strcpy(buff3,menu_button_text[t->buttontextEnum3][0]);
					w3 = UI_ProportionalStringWidth( buff3,t->style);
				}
			}

			// Which string is the longest
			if ((w > w2) && (w > w3))
			{
				;
			}
			else if ((w2 > w) && (w2 > w3))
			{
				w = w2;
			}
			else if ((w3 > w) && (w3 > w2))
			{
				w = w3;
			}
		}

		// Normal text
		else if (t->normaltextEnum)
		{
			w = w2 = w3 = 0;

			// Get longest string for width 
			if (menu_normal_text[t->normaltextEnum])
			{
				strcpy(buff,menu_normal_text[t->normaltextEnum]);
				w = UI_ProportionalStringWidth( buff,t->style);
			}

			if (t->normaltextEnum2)
			{
				if (menu_normal_text[t->normaltextEnum2])
				{
					strcpy(buff2,menu_normal_text[t->normaltextEnum2]);
					w2 = UI_ProportionalStringWidth( buff2,t->style);
				}
			}

			if (t->normaltextEnum3)
			{
				if (menu_normal_text[t->normaltextEnum3])
				{
					strcpy(buff3,menu_normal_text[t->normaltextEnum3]);
					w3 = UI_ProportionalStringWidth( buff3,t->style);
				}
			}

			// Which string is the longest
			if ((w > w2) && (w > w3))
			{
				;
			}
			else if ((w2 > w) && (w2 > w3))
			{
				w = w2;
			}
			else if ((w3 > w) && (w3 > w2))
			{
				w = w3;
			}
		}
		// possible label
		else if (t->generic.name)
		{
			strcpy(buff,t->generic.name);

			// Get width
			w = UI_ProportionalStringWidth( buff,t->style);
		}

		x = t->generic.x;
		y = t->generic.y;


		lines = 0;
		if (buff2[0] != '\0')
		{
			++lines;
			if (buff3[0] != '\0')
			{
				++lines;
			}
		}

		// Get height
		if (t->style & UI_TINYFONT)
		{
			h = PROP_TINY_HEIGHT;
		}
		else if (t->style & UI_SMALLFONT)
		{
			h = SMALLCHAR_HEIGHT;
		}
		else if (t->style & UI_BIGFONT)
		{
			h = PROP_BIG_HEIGHT;
		}
		else
		{
			h = SMALLCHAR_HEIGHT;
		}

		h += (lines * (h * 1.25));
	}

	if (t->generic.flags & QMF_RIGHT_JUSTIFY)
	{
		x = x - w;
	}
	else if (t->generic.flags & QMF_CENTER_JUSTIFY)
	{
		x = x - w/2;
	}

	t->generic.left   = x;
	t->generic.right  = x + w;
	t->generic.top    = y;
	t->generic.bottom = y + h;

}

/*
===============
Text_Draw
===============
*/
static void Text_Draw(menuframework_s *menu, menutext_s *t )
{
	int		x;
	int		y,incY;
	char	buff[512];	
	char	buff2[512];	
	char	buff3[512];	
	int		color;

	x = t->generic.x;
	y = t->generic.y;

	buff[0] = '\0';
	buff2[0] = '\0';
	buff3[0] = '\0';

	// Button text (text can be clicked on)
	if (t->buttontextEnum)
	{
		strcpy(buff,menu_button_text[t->buttontextEnum][0]);

		if (t->buttontextEnum2)
		{
			strcpy(buff2,menu_button_text[t->buttontextEnum2][0]);
		}

		if (t->buttontextEnum3)
		{
			strcpy(buff3,menu_button_text[t->buttontextEnum3][0]);
		}
	}
	// Normal text
	else if (t->normaltextEnum)
	{
		strcpy(buff,menu_normal_text[t->normaltextEnum]);
		if (t->normaltextEnum2)
		{
			strcpy(buff2,menu_normal_text[t->normaltextEnum2]);
		}

		if (t->normaltextEnum3)
		{
			strcpy(buff3,menu_normal_text[t->normaltextEnum3]);
		}
	}
	// possible label
	else if (t->generic.name)
	{
		strcpy(buff,t->generic.name);
	}

	// possible value
	if (t->string)
	{
		strcat(buff,t->string);
	}

	// Is the text grayed
	if (t->generic.flags & QMF_GRAYED)
	{
		color = CT_DKGREY;
	}
	else if (((t->generic.flags & QMF_HIGHLIGHT_IF_FOCUS) && (Menu_ItemAtCursor( t->generic.parent ) == t)) ||
		(t->generic.flags & QMF_HIGHLIGHT))
	{
		if (t->color2)
		{
			color = t->color2;
		}
		else
		{
			color = t->color;
		}

		// If there's a description for this text, print it
		if (t->buttontextEnum)
		{
			if (menu_button_text[t->buttontextEnum][1])
			{
				UI_DrawProportionalString( menu->descX, menu->descY, menu_button_text[t->buttontextEnum][1], UI_LEFT|UI_TINYFONT, colorTable[CT_BLACK]);
			}
		}
	}
	else
	{
		color = t->color;
	}

	UI_DrawProportionalString( x, y, buff, t->style, colorTable[color] );

	if (t->style & UI_TINYFONT)
	{
		incY=PROP_TINY_HEIGHT * 1.15;
	}
	else if (t->style & UI_SMALLFONT)
	{
		incY=PROP_HEIGHT * 1.15;
	}
	else if (t->style & UI_BIGFONT)
	{
		incY=PROP_BIG_HEIGHT * 1.15;
	}

	// A second line?
	if (buff2[0]!='\0')
	{
		y +=incY;
		UI_DrawProportionalString( x, y, buff2, t->style, colorTable[color] );
	}

	// A third?
	if (buff3[0]!='\0')
	{
		y +=incY;
		UI_DrawProportionalString( x, y, buff3, t->style, colorTable[color] );
	}

}

/*
===============
Bitmap_Init
===============
*/
static void Bitmap_Init( menubitmap_s *b )
{
	int	x;
	int	y;
	int	w;
	int	h;

	// Bitmap will dictate the focus area 
	if (!b->focusWidth)
	{
		x = b->generic.x;
		y = b->generic.y;
		w = b->width;
		h =	b->height;
	}
	// Focus area has a special size
	else 
	{
		x = b->focusX;
		y = b->focusY;
		w = b->focusWidth;
		h =	b->focusHeight;
	}

	if (b->generic.flags & QMF_RIGHT_JUSTIFY)
	{
		x = x - w;
	}
	else if (b->generic.flags & QMF_CENTER_JUSTIFY)
	{
		x = x - w/2;
	}

	b->generic.left   = x;
	b->generic.right  = x + w;
	b->generic.top    = y;
	b->generic.bottom = y + h;

	b->shader      = 0;
	b->focusshader = 0;
}

/*
===============
Bitmap_Draw
===============
*/
static void Bitmap_Draw( menuframework_s *menu,int index )
{
	float	x;
	float	y;
	float	w;
	float	h;
	int		color,highlight;
	menubitmap_s *b;
	int		textStyle,incY;

	b = (menubitmap_s *) menu->items[index];

	textStyle = b->textStyle;
	if (!textStyle)
	{
		textStyle = UI_LEFT|UI_SMALLFONT;
	}

	highlight = 0;

	x = b->generic.x;
	y = b->generic.y;
	w = b->width;
	h =	b->height;

	if (b->generic.flags & QMF_RIGHT_JUSTIFY)
	{
		x = x - w;
	}
	else if (b->generic.flags & QMF_CENTER_JUSTIFY)
	{
		x = x - w/2;
	}

	// used to refresh shader
	if ((b->generic.name && !b->shader) || (b->focuspic && !b->focusshader))
	{
		if (b->generic.name)
			b->shader = ui.R_RegisterShaderNoMip( b->generic.name );

		if (b->focuspic)
			b->focusshader = ui.R_RegisterShaderNoMip( b->focuspic );

	}

	// GRAYED out?
	if (b->generic.flags & QMF_GRAYED)
	{
		ui.R_SetColor( colorTable[CT_DKGREY] );
		UI_DrawHandlePic( x, y, w, h, b->shader );
		ui.R_SetColor( NULL );
	}
	// If focused on, highlight this one
	else if (((b->generic.flags & QMF_HIGHLIGHT_IF_FOCUS) && (Menu_ItemAtCursor( b->generic.parent ) == b)) ||
		(b->generic.flags & QMF_HIGHLIGHT))
	{	
		if (b->color2)
		{
			ui.R_SetColor( colorTable[b->color2]);
			highlight = 1;
		}

		UI_DrawHandlePic( x, y, w, h, b->shader );

		// If there's a description for this bitmap, print it
		if (menu_button_text[b->textEnum][1])
		{
			UI_DrawProportionalString( menu->descX, menu->descY, menu_button_text[b->textEnum][1], UI_LEFT|UI_TINYFONT, colorTable[CT_BLACK]);
		}

		ui.R_SetColor( NULL );
//		pulse_color[3] = 0.5+0.5*sin(uis.realtime/PULSE_DIVISOR);
//		ui.R_SetColor( pulse_color );
//		UI_DrawHandlePic( x, y, w, h, b->focusshader );
//		ui.R_SetColor( NULL );
	}
	// If focused on, use focus shader
	else if ((b->generic.flags & QMF_HIGHLIGHTIFFOCUS2) && (Menu_ItemAtCursor( b->generic.parent ) == b))
	{	
		UI_DrawHandlePic( x, y, w, h, b->shader );
		UI_DrawHandlePic( x, y, w, h, b->focusshader );
	}
	// Highlight 
	else if (b->generic.flags & QMF_HIGHLIGHT)
	{
//		UI_DrawHandlePic( x, y, w, h, b->shader );
//		UI_DrawHandlePic( x, y, w, h, b->focusshader );
	} 
	else if ( b->generic.flags & QMF_BLINK )
	{
		if ( (uis.realtime/BLINK_DIVISOR) & 1 )
		{
			ui.R_SetColor( colorTable[b->color]);
		}
		else
		{
			ui.R_SetColor( colorTable[b->color2]);
		}
		UI_DrawHandlePic( x, y, w, h, b->shader );
		ui.R_SetColor( NULL );
	}
	// All others
	else
	{
		if (b->color)
		{
			ui.R_SetColor( colorTable[b->color]);
		}
		UI_DrawHandlePic( x, y, w, h, b->shader );
		ui.R_SetColor( NULL );
	}

	// If there's text to go on top of the bitmap
	if ((b->textEnum) || (b->textPtr))
	{
		// Bitmap is highlighted, use textcolor2
		if (highlight)
		{
			color = ( b->textcolor2 ) ? b->textcolor2 : CT_WHITE;
		}
		// Bitmap is not highlighted, use textcolor
		else
		{
			color = ( b->textcolor ) ? b->textcolor : CT_WHITE;
		}

		// Draw button text
		if ((b->textEnum) && (menu_button_text[b->textEnum][0]))
		{
			UI_DrawProportionalString( x + b->textX, y + b->textY , menu_button_text[b->textEnum][0], textStyle, colorTable[color]);
		}
		else if (b->textPtr)
		{
			UI_DrawProportionalString( x + b->textX, y + b->textY , b->textPtr, textStyle, colorTable[color]);
		}


		// Is there a 2nd line of button text?
		if (b->textEnum2)
		{
			if (textStyle & UI_SMALLFONT)
			{
				incY=PROP_HEIGHT * 1.15;
			}
			else if (textStyle & UI_TINYFONT)
			{
				incY=PROP_TINY_HEIGHT * 1.15;
			}

			// Print second line of text
			UI_DrawProportionalString( x + b->textX, y + b->textY + incY , menu_button_text[b->textEnum2][0], textStyle, colorTable[color]);
		}
	}
}

/*
===============
Action_Init
===============
*/
void Action_Init( menuaction_s *a )
{
	int x,y,w,h;
/*	int	len;

	// calculate bounds
	if (a->generic.name)
		len = strlen(a->generic.name);
	else
		len = 0;

	if (a->generic.flags & QMF_RIGHT_JUSTIFY)
	{
		// right justify text
		a->generic.left   = a->generic.x - len*BIGCHAR_WIDTH; 
		a->generic.right  = a->generic.x;
	}
	else if (a->generic.flags & QMF_CENTER_JUSTIFY)
	{
		// center justify text
		a->generic.left   = a->generic.x - len*BIGCHAR_WIDTH/2; 
		a->generic.right  = a->generic.x + len*BIGCHAR_WIDTH/2;
	}
	else
	{
		// left justify text
		a->generic.left   = a->generic.x; 
		a->generic.right  = a->generic.x + len*BIGCHAR_WIDTH;
	}	

	a->generic.top    = a->generic.y;
	a->generic.bottom = a->generic.y + BIGCHAR_HEIGHT;
*/

	x = a->generic.x;
	y = a->generic.y;
	w = a->width;
	h =	a->height;

	if (a->generic.flags & QMF_RIGHT_JUSTIFY)
	{
		x = x - w;
	}
	else if (a->generic.flags & QMF_CENTER_JUSTIFY)
	{
		x = x - w/2;
	}

	a->generic.left   = x;
	a->generic.right  = x + w;
	a->generic.top    = y;
	a->generic.bottom = y + h;
}

/*
===============
Action_Focus
===============
*/
void Action_Focus( menuaction_s *a )
{
}

/*
===============
Action_Draw
===============
*/
void Action_Draw( menuaction_s *a )
{
	int x, y,textColor,buttonColor;
	int	style;
	int		incY;

	//fixme: this shouldn't be happneing!
	if (!*menu_button_text[a->textEnum][0]) {	//don't have a string for some reason...
		return;
	}
	if ( a->generic.flags & QMF_GRAYED )
	{
		buttonColor = a->color3;
		textColor = a->textcolor3;
	}
	else if (( a->generic.flags & QMF_HIGHLIGHT_IF_FOCUS ) && ( a->generic.parent->cursor == a->generic.menuPosition ))
	{
		buttonColor = a->color2;
		textColor = a->textcolor2;
		UI_DrawProportionalString( a->generic.parent->descX, a->generic.parent->descY, menu_button_text[a->textEnum][1], UI_LEFT|UI_TINYFONT, colorTable[CT_BLACK]);

//		UI_LerpColor(menu_highlight_color,menu_lowlight_color,menucolor,0.5+0.5*sin(uis.realtime/PULSE_DIVISOR));
	}
	else if ( a->generic.flags & QMF_HIGHLIGHTIFFOCUS2 )
	{
		// steady focus
		if ( a->generic.parent->cursor == a->generic.menuPosition )
		{
			buttonColor = a->color2;
			textColor = a->textcolor2;
		}
		else
		{
			buttonColor = a->color;
			textColor = a->textcolor;
		}
	}
	else if ( a->generic.flags & QMF_BLINK )
	{
		if ( (uis.realtime/BLINK_DIVISOR) & 1 )
		{
			buttonColor = a->color;
			textColor = a->textcolor;
		}
		else
		{
			buttonColor = a->color2;
			textColor = a->textcolor2;
		}
	}
	else	// Just plain old normal
	{
		buttonColor = a->color2;
		textColor = a->textcolor2;
//		color = menucolor;
//		UI_LerpColor(menu_text_color,menu_highlight_color,menucolor,a->generic.decay);
	}

	x = a->generic.x;

	y = a->generic.y;

	if ( a->generic.flags & QMF_RIGHT_JUSTIFY )
		style = UI_RIGHT;
	else if ( a->generic.flags & QMF_CENTER_JUSTIFY )
		style = UI_CENTER;
	else
		style = UI_LEFT;


	ui.R_SetColor( colorTable[buttonColor]);
	UI_DrawHandlePic(x,y, a->width, a->height, uis.whiteShader);

	// Draw button text
	if (menu_button_text[a->textEnum][0])
	{
		UI_DrawProportionalString( x + a->textX, y + a->textY, menu_button_text[a->textEnum][0], style | UI_SMALLFONT, colorTable[textColor] );
	}

	// Is there a 2nd line of text?
	if (a->textEnum2)
	{
		if (style & UI_SMALLFONT)
		{
			incY=PROP_HEIGHT * 1.15;
		}
		else if (style & UI_TINYFONT)
		{
			incY=PROP_TINY_HEIGHT * 1.15;
		}

		// Print second line of text
		UI_DrawProportionalString( x + a->textX, y + a->textY + incY , menu_button_text[a->textEnum2][0], style, colorTable[textColor]);
	}

}

/*
===============
ListBox_Init
===============
*/
static void ListBox_Init( menulist_s *l )
{
	l->generic.left   =	l->generic.x;
	l->generic.top    = l->generic.y;	
	l->generic.right  =	l->generic.x + (l->width+2)*BIGCHAR_WIDTH;
	l->generic.bottom =	l->generic.y + (l->height+2)*BIGCHAR_HEIGHT;

	l->top = 0;
}

/*
===============
ListBox_Focus
===============
*/
static void ListBox_Focus( menulist_s *l )
{
	int		x;
	int		y;
	int	cursorx;
	int	cursory;

	y =	l->generic.y + BIGCHAR_HEIGHT;
	cursory = (uis.cursory - y)/BIGCHAR_HEIGHT;

	if ((cursory < 0) || (cursory >= l->height))
		return;

	x =	l->generic.x + BIGCHAR_WIDTH;
	cursorx = (uis.cursorx - x)/BIGCHAR_WIDTH;

	if ((cursorx >= 0) && (cursorx < l->width))
		l->curvalue = l->top + cursory;
}

/*
===============
ListBox_Key
===============
*/
static sfxHandle_t ListBox_Key( menulist_s *l, int key )
{
	int	x;
	int	y;

	switch (key)
	{
		case K_MOUSE1:
			if (l->generic.flags & QMF_HASMOUSEFOCUS)
			{
				// check up arrow
				x = l->generic.x + (l->width+1)*BIGCHAR_WIDTH;
				y  = l->generic.y + BIGCHAR_HEIGHT;
				if (UI_CursorInRect( x, y, BIGCHAR_WIDTH, BIGCHAR_HEIGHT ))
				{
					goto uparrow;
				}

				// check down arrow
				y = l->generic.y + l->height*BIGCHAR_HEIGHT;
				if (UI_CursorInRect( x, y, BIGCHAR_WIDTH, BIGCHAR_HEIGHT ))
				{
					goto downarrow;
				}

				// check non active regions
				x = l->generic.x + BIGCHAR_WIDTH;
				y = l->generic.y + BIGCHAR_HEIGHT;
				if (!UI_CursorInRect( x, y, l->width*BIGCHAR_WIDTH, l->height*BIGCHAR_HEIGHT ))
				{
					return (menu_buzz_sound);
				}
			}
			break;

		case K_KP_HOME:
		case K_HOME:
			l->top = 0;
			l->curvalue = 0;
			return (menu_move_sound);

		case K_KP_END:
		case K_END:
			l->curvalue = l->numitems-1;
			l->top = l->curvalue - (l->height-1);
			if (l->top < 0)
				l->top = 0;			
			return (menu_move_sound);

uparrow:
		case K_KP_UPARROW:
		case K_UPARROW:
			if (l->curvalue > 0 && l->curvalue >= l->top)
			{
				l->curvalue--;
				if (l->curvalue < l->top)
					l->top = l->curvalue;
				return (menu_move_sound);
			}
			return (menu_buzz_sound);
downarrow:
		case K_KP_DOWNARROW:
		case K_DOWNARROW:
			if (l->curvalue < l->numitems-1 && l->curvalue < l->top + l->height)
			{
				l->curvalue++;
				if (l->curvalue >= l->top + l->height)
					l->top++;
				return (menu_move_sound);
			}
			return (menu_buzz_sound);
	}

	// event not handled, pass to default handler
	return (NULL);
}

/*
===============
ListBox_Draw
===============
*/
static void ListBox_Draw( menulist_s *l )
{
	int		i;
	int		n;
	int		x;
	int		y;
	int		cx;
	int		cy;
	int		width;
	float	*color;
	int		style;

	x =	l->generic.x;
	y =	l->generic.y;

	// draw left side
	cx = x;
	cy = y;
	UI_DrawChar( cx, cy, 1, UI_LEFT|UI_BIGFONT, menu_text_color );
	for (n = 0; n < l->height; n++)
	{
		cy += BIGCHAR_HEIGHT;
		UI_DrawChar( cx, cy, 4, UI_LEFT|UI_BIGFONT, menu_text_color );
	}
	UI_DrawChar( cx, cy+BIGCHAR_HEIGHT, 7, UI_LEFT|UI_BIGFONT, menu_text_color);

	// draw middle
	width = l->width;
	cx += BIGCHAR_WIDTH;
	while (width > 0)
	{
		cy = y;
		UI_DrawChar( cx, cy, 2, UI_LEFT|UI_BIGFONT, menu_text_color );
		for (n = 0; n < l->height; n++)
		{
			cy += BIGCHAR_HEIGHT;
			UI_DrawChar( cx, cy, 5, UI_LEFT|UI_BIGFONT, menu_text_color);
		}
		UI_DrawChar(cx, cy+BIGCHAR_HEIGHT, 8, UI_LEFT|UI_BIGFONT, menu_text_color);
		width -= 1;
		cx += BIGCHAR_WIDTH;
	}

// draw right side
	cy = y;
	UI_DrawChar(cx, cy, 3, UI_LEFT|UI_BIGFONT, menu_text_color);
	cy += 2*BIGCHAR_HEIGHT;
	for (n = 0; n < l->height-2; n++)
	{
		UI_DrawChar(cx, cy, 15, UI_LEFT|UI_BIGFONT, menu_text_color);
		cy += BIGCHAR_HEIGHT;
	}
	UI_DrawChar(cx, cy + BIGCHAR_HEIGHT, 9, UI_LEFT|UI_BIGFONT, menu_text_color);

	// draw up arrow
	if (UI_CursorInRect(cx,y+BIGCHAR_HEIGHT,BIGCHAR_WIDTH,BIGCHAR_HEIGHT))
	{
		style = UI_LEFT|UI_PULSE;
		color = menu_highlight_color;
	}
	else
	{
		style = UI_LEFT;
		color = menu_text_color;
	}
	UI_DrawChar(cx, y + BIGCHAR_HEIGHT, 135, style, color);

	// draw down arrow
	if (UI_CursorInRect(cx,cy,BIGCHAR_WIDTH,BIGCHAR_HEIGHT))
	{
		style = UI_LEFT|UI_PULSE;
		color = menu_highlight_color;
	}
	else
	{
		style = UI_LEFT;
		color = menu_text_color;
	}
	UI_DrawChar(cx, cy, 134, style, color);

	// draw thumb
	if (l->numitems > l->height)
		n = (((l->height-3)*l->top)/(l->numitems-l->height)+2)*BIGCHAR_HEIGHT;
	else
		n = 2*BIGCHAR_HEIGHT;
	UI_DrawChar(cx, y + n, 6, UI_LEFT|UI_BIGFONT, menu_text_color);

	if ( l->generic.name )
	{	
		// title plaque
		cx = x + (l->width+2)*BIGCHAR_WIDTH/2 - (strlen(l->generic.name)+2)*BIGCHAR_WIDTH/2;		
		UI_FillRect(cx,y,(strlen(l->generic.name)+2)*BIGCHAR_WIDTH,BIGCHAR_HEIGHT,g_color_table[ColorIndex(COLOR_BLACK)]);

		UI_DrawString(
			cx+BIGCHAR_WIDTH,y-2,
			l->generic.name,
			UI_LEFT,
			menu_text_color);
	}

	// iterate the list box
	x += BIGCHAR_WIDTH;
	y += BIGCHAR_HEIGHT;
	for (i=l->top; i<l->top+l->height; i++) {
		if (i >= l->numitems)
			break;

		if (i == l->curvalue)
		{
			UI_FillRect(x,y+2,l->width*BIGCHAR_WIDTH,BIGCHAR_HEIGHT,menu_highlight_color);
			UI_FillRect(x+1,y+3,l->width*BIGCHAR_WIDTH-2,BIGCHAR_HEIGHT-2,g_color_table[ColorIndex(COLOR_BLACK)]);
			color = menu_highlight_color;
			style = UI_LEFT|UI_PULSE;
		}
		else
		{
			style = UI_LEFT;
			color = menu_text_color;
		}

		UI_DrawString(
			l->generic.x + BIGCHAR_WIDTH, 
			y,
			l->itemnames[i],
			style,
			color);

		y += BIGCHAR_HEIGHT;
	}
}

/*
===============
RadioButton_Init
===============
*/
void RadioButton_Init( menuradiobutton_s *rb )
{
	int	len;
	int	w;
	int	h;

	// calculate bounds
	if ( rb->generic.name )
		len = strlen(rb->generic.name);
	else
		len = 0;

	if (rb->generic.flags & QMF_SMALLFONT)
	{
		w = SMALLCHAR_WIDTH;
		h = SMALLCHAR_HEIGHT;
	}
	else
	{
		w = BIGCHAR_WIDTH;
		h = BIGCHAR_HEIGHT;
	}

	rb->generic.left   = rb->generic.x - (len+1)*w;
	rb->generic.right  = rb->generic.x + 6*w;
	rb->generic.top    = rb->generic.y;
	rb->generic.bottom = rb->generic.y + h;
}

/*
===============
RadioButton_Key
===============
*/
static sfxHandle_t RadioButton_Key( menuradiobutton_s *rb, int key )
{
	switch (key)
	{
		case K_MOUSE1:
			if (!(rb->generic.flags & QMF_HASMOUSEFOCUS))
				break;

		case K_JOY1:
		case K_JOY2:
		case K_JOY3:
		case K_JOY4:
		case K_ENTER:
		case K_KP_ENTER:
		case K_KP_LEFTARROW:
		case K_LEFTARROW:
		case K_KP_RIGHTARROW:
		case K_RIGHTARROW:
			rb->curvalue = !rb->curvalue;
			if ( rb->generic.callback )
				rb->generic.callback( rb, QM_ACTIVATED );

			return (menu_move_sound);
	}

	// key not handled
	return (NULL);
}

/*
===============
RadioButton_Draw
===============
*/
static void RadioButton_Draw( menuradiobutton_s *rb )
{
	int	x;
	int y;
	int	w;
	int	h;
	float *labelColor;
	float *color;
	int	style;
	int	size;

	if (rb->generic.flags & QMF_SMALLFONT)
	{
		w    = SMALLCHAR_WIDTH;
		h    = SMALLCHAR_HEIGHT;
		size = UI_SMALLFONT;
	}
	else
	{
		w    = BIGCHAR_WIDTH;
		h    = BIGCHAR_HEIGHT;
		size = UI_BIGFONT;
	}

	x = rb->generic.x;

	y = rb->generic.y;

	if ( rb->generic.flags & QMF_GRAYED )
	{
		labelColor = menu_grayed_color;
		color      = menu_grayed_color;
		style      = UI_LEFT;
	}
	else if (Menu_ItemAtCursor( rb->generic.parent ) == rb)
	{
		labelColor = menu_highlight_color;
		color      = menu_highlight_color;
		style      = UI_LEFT|UI_PULSE;
	}
	else
	{
		labelColor = menu_dark_color;
		color      = menu_text_color;
		style      = UI_LEFT;
	}

	if ( rb->generic.name )
	{
		UI_DrawString( x - w, y, rb->generic.name, UI_RIGHT|size, labelColor );
	}

	if ( !rb->curvalue )
	{
		UI_DrawNamedPic( x + w, y + 2, 16, 16, "menu/art/switch_off");
		UI_DrawString( x + w + 16, y, "off", style|size, color );
		
	}
	else
	{
		UI_DrawNamedPic( x + w, y + 2, 16, 16, "menu/art/switch_on");
		UI_DrawString( x + w + 16, y, "on", style|size, color );
	}

	// draw cursor
	if (Menu_ItemAtCursor( rb->generic.parent ) == rb)
	{
		UI_DrawChar( x, y, 13, UI_CENTER|UI_BLINK|size, labelColor);
	}
}

/*
===============
Separator_Draw
===============
*/
void Separator_Draw( menuseparator_s *s )
{
	int y;
	int	style;

	if ( s->generic.name )
	{
		y = s->generic.y;

		if (s->generic.flags & QMF_RIGHT_JUSTIFY)
			style = UI_RIGHT;
		else if (s->generic.flags & QMF_CENTER_JUSTIFY)
			style = UI_CENTER;
		else
			style = UI_LEFT;

		UI_DrawString( s->generic.x, y, s->generic.name, style, menu_dark_color );
	}
}

/*
===============
Slider_Init
===============
*/
static void Slider_Init( menuslider_s *s )
{

	// Labels go on the left side.
	if (s->picX)
	{
		s->generic.left = s->picX;
		s->generic.right = s->generic.x + s->focusWidth;
		s->generic.top  = s->picY;
		s->generic.bottom = s->generic.y + s->focusHeight;
	}
	else
	{
		s->generic.left = s->generic.x;
		s->generic.right = s->generic.x + s->focusWidth;
		s->generic.top    = s->generic.y;
		s->generic.bottom = s->generic.y + s->focusHeight;
	}

	if (s->thumbName)
	{
		s->thumbShader = ui.R_RegisterShaderNoMip( s->thumbName);
	}

	if (s->picName)
	{
		s->picShader = ui.R_RegisterShaderNoMip( s->picName);
	}

	if (s->generic.name)
	{
		s->shader = ui.R_RegisterShaderNoMip( s->generic.name);
	}
}

/*
===============
Slider_Key
===============
*/
static sfxHandle_t Slider_Key( menuslider_s *s, int key )
{
	sfxHandle_t sound;
	int			xLength;
	int			oldvalue;

	switch (key)
	{
		case K_MOUSE1:
			xLength     = uis.cursorx - s->generic.x;
			oldvalue    = s->curvalue;
			s->curvalue = (xLength/(float)(s->focusWidth)) * (s->maxvalue-s->minvalue) + s->minvalue;

			if (s->curvalue < s->minvalue)
			{
				s->curvalue = s->minvalue;
			}
			else if (s->curvalue > s->maxvalue)
			{
				s->curvalue = s->maxvalue;
			}

			if (s->curvalue != oldvalue)
			{
				sound = menu_move_sound;
			}
			else
			{
				sound = NULL;
			}
			break;

		case K_KP_LEFTARROW:
		case K_LEFTARROW:
			if (s->curvalue > s->minvalue)
			{
				s->curvalue--;
				sound = menu_move_sound;
			}
			else
				sound = menu_buzz_sound;
			break;			

		case K_KP_RIGHTARROW:
		case K_RIGHTARROW:
			if (s->curvalue < s->maxvalue)
			{
				s->curvalue++;
				sound = menu_move_sound;
			}
			else
				sound = menu_buzz_sound;
			break;			

		default:
			// key not handled
			sound = NULL;
			break;
	}

	if ( sound && s->generic.callback )
		s->generic.callback( s, QM_ACTIVATED );

	return (sound);
}

/*
===============
Slider_Draw
===============
*/
static void Slider_Draw( menuslider_s *s )
{
	int color;
	int	style;
	int textColor;
	int	thumbColor,backgroundColor;
	int	focus,thumbX;
	

	focus = 0;

	if ( s->generic.flags & QMF_GRAYED )
	{
		color = CT_DKGREY;
		style = UI_LEFT;
	}
	else if (Menu_ItemAtCursor( s->generic.parent ) == s)
	{
		color = s->color2;
		style = UI_LEFT|UI_PULSE;
		textColor = s->textcolor2;
		thumbColor = s->thumbColor2;
		backgroundColor = CT_LTGREY;
		focus = 1;
	}
	else
	{
		color = s->color;
		style = UI_LEFT;
		textColor = s->textcolor;
		thumbColor = s->thumbColor;
		backgroundColor = CT_DKGREY;
	}

	// Print pic
	if (s->picShader)
	{
		ui.R_SetColor( colorTable[color]);
		UI_DrawHandlePic( s->picX , s->picY,  s->picWidth,  s->picHeight, s->picShader);
		ui.R_SetColor( NULL );
	}

	// Button text
	if (s->textEnum)
	{
		UI_DrawProportionalString(  s->picX + s->textX,  s->picY + s->textY,menu_button_text[s->textEnum][0], UI_SMALLFONT, colorTable[textColor] );
	}

	if ((focus) && (menu_button_text[s->textEnum][1]))
	{
		UI_DrawProportionalString( s->generic.parent->descX, s->generic.parent->descY, menu_button_text[s->textEnum][1], UI_LEFT|UI_TINYFONT, colorTable[CT_BLACK]);
	}

	// clamp thumb
	if (s->maxvalue > s->minvalue)
	{
		s->range = ( s->curvalue - s->minvalue ) / ( float ) ( s->maxvalue - s->minvalue );
		if ( s->range < 0)
		{
			s->range = 0;
		}
		else if ( s->range > 1)
		{
			s->range = 1;
		}
	}
	else
	{
		s->range = 0;
	}

	thumbX = s->generic.x - (s->thumbGraphicWidth / 2) + (s->focusWidth * s->range);
	if (s->range)
	{
		ui.R_SetColor( colorTable[backgroundColor]);
		UI_DrawHandlePic( s->generic.x, s->generic.y, (thumbX - s->generic.x) + 4, 18, uis.whiteShader); // Hard coded height is bad
	}

	//draw slider graph
	if (s->shader)
	{
		ui.R_SetColor( colorTable[color]);
		UI_DrawHandlePic( s->generic.x, s->generic.y, s->width, s->height, s->shader); 
	}

	// Draw thumb and background
	if (s->thumbShader)
	{
		ui.R_SetColor( colorTable[thumbColor]);
		UI_DrawHandlePic( thumbX, s->generic.y, s->thumbWidth, s->thumbHeight, s->thumbShader); 
	}

}

/*
===============
SpinControl_Init
===============
*/
void SpinControl_Init( menulist_s *s ) 
{
	// Count number of choices for this spin control
	s->numitems=0;

	if (s->listnames)
	{
		while ( (menu_normal_text[s->listnames[s->numitems]]) != 0 )
		{
			s->numitems++;
		}
	}
	else
	{
		while ( s->itemnames[s->numitems] != 0 )
		{
			s->numitems++;
		}
	}

	if (!s->width)
	{
		s->width = MENU_BUTTON_MED_WIDTH;
	}

	s->generic.left		= s->generic.x;
	s->generic.right	= s->generic.x + MENU_BUTTON_MED_HEIGHT + s->width + MENU_BUTTON_MED_HEIGHT;
	s->generic.top		= s->generic.y;
	s->generic.bottom	= s->generic.y + MENU_BUTTON_MED_HEIGHT;
}

/*
===============
SpinControl_Key
===============
*/
static sfxHandle_t SpinControl_Key( menulist_s *s, int key )
{
	sfxHandle_t sound;

	sound = NULL;
	switch (key)
	{
		case K_MOUSE1:
			if (!(s->generic.flags & QMF_HASMOUSEFOCUS))
			{
				break;
			}

		case K_ENTER:
		case K_KP_ENTER:
			s->curvalue++;
			if (s->curvalue >= s->numitems)
				s->curvalue = 0;
			sound = menu_move_sound;
			break;
		
		case K_KP_LEFTARROW:
		case K_LEFTARROW:
			if (s->curvalue > 0)
			{
				s->curvalue--;
				sound = menu_move_sound;
			}
			else
				sound = menu_buzz_sound;
			break;

		case K_KP_RIGHTARROW:
		case K_RIGHTARROW:
			if (s->curvalue < s->numitems-1)
			{
				s->curvalue++;
				sound = menu_move_sound;
			}
			else
				sound = menu_buzz_sound;
			break;
	}

	if ( sound && s->generic.callback )
		s->generic.callback( s, QM_ACTIVATED );

	return (sound);
}

/*
===============
SpinControl_Draw
===============
*/
void SpinControl_Draw( menulist_s *s )
{
	int x,y,listX,buttonColor,buttonTextColor;

	x = s->generic.x;
	y =	s->generic.y;

	// Print current value
	if (s->listnames)
	{
		if ( !strchr( menu_normal_text[s->listnames[s->curvalue]], '\n' ) )
		{
			listX = x + MENU_BUTTON_MED_HEIGHT + s->width - 8 + MENU_BUTTON_MED_HEIGHT + 4;
			UI_DrawProportionalString( listX, y + s->textY,menu_normal_text[s->listnames[s->curvalue]], UI_SMALLFONT, colorTable[CT_WHITE] );
		}
	}
	else if (s->itemnames[0])
	{
		if ( !strchr(s->itemnames[s->curvalue], '\n' ) )
		{
			listX = x + MENU_BUTTON_MED_HEIGHT + s->width - 8 + MENU_BUTTON_MED_HEIGHT + 4;
			UI_DrawProportionalString( listX, y + s->textY,s->itemnames[s->curvalue], UI_SMALLFONT, colorTable[CT_WHITE] );
		}
	}

	// Choose colors
	if (s->generic.flags & QMF_GRAYED)
	{
		buttonColor = CT_DKGREY;
		buttonTextColor = s->textcolor;
	}
	else if (Menu_ItemAtCursor( s->generic.parent ) == s)
	{
		// Button description
		if (menu_button_text[s->textEnum][1])
		{
			UI_DrawProportionalString( s->generic.parent->descX, s->generic.parent->descY, menu_button_text[s->textEnum][1], UI_LEFT|UI_TINYFONT, colorTable[CT_BLACK]);
		}

		buttonColor = s->color2;
		buttonTextColor = s->textcolor2;
	}
	else
	{
		buttonColor = s->color;
		buttonTextColor = s->textcolor;
	}

	// Print little dot to show it was updated
//	if (s->updated)
//	{
//		ui.R_SetColor( colorTable[CT_LTGOLD1]);
//		UI_DrawHandlePic(x - 10,y + 6, 8, 8, uis.graphicCircle);
//	}

	// Draw button and button text
	ui.R_SetColor( colorTable[buttonColor]);
	UI_DrawHandlePic(x,y, MENU_BUTTON_MED_HEIGHT, MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);											// Left
	UI_DrawHandlePic(x+ s->width+ MENU_BUTTON_MED_HEIGHT - 16, y, -MENU_BUTTON_MED_HEIGHT, MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);	// Right
	UI_DrawHandlePic(x + MENU_BUTTON_MED_HEIGHT - 8,y, s->width, MENU_BUTTON_MED_HEIGHT, uis.whiteShader);										// Middle

	UI_DrawProportionalString( x + s->textX, y + s->textY,menu_button_text[s->textEnum][0], UI_SMALLFONT, colorTable[buttonTextColor] );
}

/*
=================
ScrollList_Init
=================
*/
static void ScrollList_Init( menulist_s *l )
{
	int		w;

	l->oldvalue = 0;
	l->curvalue = 0;
	l->top      = 0;

	if( !l->columns ) {
		l->columns = 1;
		l->seperation = 0;
	}
	else if( !l->seperation ) {
		l->seperation = 3;
	}

	w = ( (l->width + l->seperation) * l->columns - l->seperation) * SMALLCHAR_WIDTH;

	l->generic.left   =	l->generic.x;
	l->generic.top    = l->generic.y;	
	l->generic.right  =	l->generic.x + w;
	l->generic.bottom =	l->generic.y + l->height * SMALLCHAR_HEIGHT;

	if( l->generic.flags & QMF_CENTER_JUSTIFY ) {
		l->generic.left -= w / 2;
		l->generic.right -= w / 2;
	}
}

/*
===============
ScrollList_Key
===============
*/
sfxHandle_t ScrollList_Key( menulist_s *l, int key )
{
	int	x;
	int	y;
	int	w;
	int	i;
	int	j;	
	int	c;
	int	cursorx;
	int	cursory;
	int	column;
	int	index;

	switch (key)
	{
		case K_MOUSE1:
			if (l->generic.flags & QMF_HASMOUSEFOCUS)
			{
				// check scroll region
				x = l->generic.x;
				y = l->generic.y;
				w = ( (l->width + l->seperation) * l->columns - l->seperation) * SMALLCHAR_WIDTH;
				if( l->generic.flags & QMF_CENTER_JUSTIFY ) {
					x -= w / 2;
				}
				if (UI_CursorInRect( x, y, w, l->height*SMALLCHAR_HEIGHT ))
				{
					cursorx = (uis.cursorx - x)/SMALLCHAR_WIDTH;
					column = cursorx / (l->width + l->seperation);
					cursory = (uis.cursory - y)/SMALLCHAR_HEIGHT;
					index = column * l->height + cursory;
					if (l->top + index < l->numitems)
					{
						l->oldvalue = l->curvalue;
						l->curvalue = l->top + index;

						if (l->oldvalue != l->curvalue && l->generic.callback)
						{
							l->generic.callback( l, QM_GOTFOCUS );
							return (menu_move_sound);
						}
					}
				}
			
				// absorbed, silent sound effect
				return (menu_null_sound);
			}
			break;

		case K_KP_HOME:
		case K_HOME:
			l->oldvalue = l->curvalue;
			l->curvalue = 0;
			l->top      = 0;

			if (l->oldvalue != l->curvalue && l->generic.callback)
			{
				l->generic.callback( l, QM_GOTFOCUS );
				return (menu_move_sound);
			}
			return (menu_buzz_sound);

		case K_KP_END:
		case K_END:
			l->oldvalue = l->curvalue;
			l->curvalue = l->numitems-1;
			if( l->columns > 1 ) {
				c = (l->curvalue / l->height + 1) * l->height;
				l->top = c - (l->columns * l->height);
			}
			else {
				l->top = l->curvalue - (l->height - 1);
			}
			if (l->top < 0)
				l->top = 0;			

			if (l->oldvalue != l->curvalue && l->generic.callback)
			{
				l->generic.callback( l, QM_GOTFOCUS );
				return (menu_move_sound);
			}
			return (menu_buzz_sound);

		case K_PGUP:
		case K_KP_PGUP:
			if( l->columns > 1 ) {
				return menu_null_sound;
			}

			if (l->curvalue > 0)
			{
				l->oldvalue = l->curvalue;
				l->curvalue -= l->height-1;
				if (l->curvalue < 0)
					l->curvalue = 0;
				l->top = l->curvalue;
				if (l->top < 0)
					l->top = 0;

				if (l->generic.callback)
					l->generic.callback( l, QM_GOTFOCUS );

				return (menu_move_sound);
			}
			return (menu_buzz_sound);

		case K_PGDN:
		case K_KP_PGDN:
			if( l->columns > 1 ) {
				return menu_null_sound;
			}

			if (l->curvalue < l->numitems-1)
			{
				l->oldvalue = l->curvalue;
				l->curvalue += l->height-1;
				if (l->curvalue > l->numitems-1)
					l->curvalue = l->numitems-1;
				l->top = l->curvalue - (l->height-1);
				if (l->top < 0)
					l->top = 0;

				if (l->generic.callback)
					l->generic.callback( l, QM_GOTFOCUS );

				return (menu_move_sound);
			}
			return (menu_buzz_sound);

		case K_KP_UPARROW:
		case K_UPARROW:
			if( l->curvalue == 0 ) {
				return menu_buzz_sound;
			}

			l->oldvalue = l->curvalue;
			l->curvalue--;

			if( l->curvalue < l->top ) {
				if( l->columns == 1 ) {
					l->top--;
				}
				else {
					l->top -= l->height;
				}
			}

			if( l->generic.callback ) {
				l->generic.callback( l, QM_GOTFOCUS );
			}

			return (menu_move_sound);

		case K_KP_DOWNARROW:
		case K_DOWNARROW:
			if( l->curvalue == l->numitems - 1 ) {
				return menu_buzz_sound;
			}

			l->oldvalue = l->curvalue;
			l->curvalue++;

			if( l->curvalue >= l->top + l->columns * l->height ) {
				if( l->columns == 1 ) {
					l->top++;
				}
				else {
					l->top += l->height;
				}
			}

			if( l->generic.callback ) {
				l->generic.callback( l, QM_GOTFOCUS );
			}

			return menu_move_sound;

		case K_KP_LEFTARROW:
		case K_LEFTARROW:
			if( l->columns == 1 ) {
				return menu_null_sound;
			}

			if( l->curvalue < l->height ) {
				return menu_buzz_sound;
			}

			l->oldvalue = l->curvalue;
			l->curvalue -= l->height;

			if( l->curvalue < l->top ) {
				l->top -= l->height;
			}

			if( l->generic.callback ) {
				l->generic.callback( l, QM_GOTFOCUS );
			}

			return menu_move_sound;

		case K_KP_RIGHTARROW:
		case K_RIGHTARROW:
			if( l->columns == 1 ) {
				return menu_null_sound;
			}

			c = l->curvalue + l->height;

			if( c >= l->numitems ) {
				return menu_buzz_sound;
			}

			l->oldvalue = l->curvalue;
			l->curvalue = c;

			if( l->curvalue > l->top + l->columns * l->height - 1 ) {
				l->top += l->height;
			}

			if( l->generic.callback ) {
				l->generic.callback( l, QM_GOTFOCUS );
			}

			return menu_move_sound;
	}

	// cycle look for ascii key inside list items
	if ( !Q_isprint( key ) )
		return (0);

	// force to lower for case insensitive compare
	if ( Q_isupper( key ) )
	{
		key -= 'A' - 'a';
	}

	// iterate list items
	for (i=1; i<=l->numitems; i++)
	{
		j = (l->curvalue + i) % l->numitems;
		c = l->itemnames[j][0];
		if ( Q_isupper( c ) )
		{
			c -= 'A' - 'a';
		}

		if (c == key)
		{
			// set current item, mimic windows listbox scroll behavior
			if (j < l->top)
			{
				// behind top most item, set this as new top
				l->top = j;
			}
			else if (j > l->top+l->height-1)
			{
				// past end of list box, do page down
				l->top = (j+1) - l->height;
			}
			
			if (l->curvalue != j)
			{
				l->oldvalue = l->curvalue;
				l->curvalue = j;
				if (l->generic.callback)
					l->generic.callback( l, QM_GOTFOCUS );
				return ( menu_move_sound );			
			}

			return (menu_buzz_sound);
		}
	}

	return (menu_buzz_sound);
}

/*
=================
ScrollList_Draw
=================
*/
void ScrollList_Draw( menulist_s *l )
{
	int			x;
	int			u;
	int			y;
	int			i;
	int			base;
	int			column;
	float*		color;
	qboolean	hasfocus;
	int			style;

	hasfocus = (l->generic.parent->cursor == l->generic.menuPosition);

	x =	l->generic.x;
	for( column = 0; column < l->columns; column++ ) {
		y =	l->generic.y;
		base = l->top + column * l->height;
		for( i = base; i < base + l->height; i++) {
			if (i >= l->numitems)
				break;

			if (i == l->curvalue)
			{
				u = x - 2;
				if( l->generic.flags & QMF_CENTER_JUSTIFY ) {
					u -= (l->width * SMALLCHAR_WIDTH) / 2 + 1;
				}

				UI_FillRect(u,y,l->width*SMALLCHAR_WIDTH,SMALLCHAR_HEIGHT+2,listbar_color);
				color = text_color_highlight;

				if (hasfocus)
					style = UI_PULSE|UI_LEFT|UI_SMALLFONT;
				else
					style = UI_LEFT|UI_SMALLFONT;
			}
			else
			{
				color = text_color_normal;
				style = UI_LEFT|UI_SMALLFONT;
			}
			if( l->generic.flags & QMF_CENTER_JUSTIFY ) {
				style |= UI_CENTER;
			}
			if( l->generic.flags & QMF_CENTER_JUSTIFY ) {
				style |= UI_CENTER;
			}

			UI_DrawString(
				x,
				y,
				l->itemnames[i],
				style,
				color);

			y += SMALLCHAR_HEIGHT;
		}
		x += (l->width + l->seperation) * SMALLCHAR_WIDTH;
	}
}

/*
===============
Menu_AddItem
===============
*/
void Menu_AddItem( menuframework_s *menu, void *item )
{
	menucommon_s	*itemptr;

	if (menu->nitems >= MAX_MENUITEMS)
		ui.Error (ERR_FATAL, "Menu_AddItem: excessive items");

	menu->items[menu->nitems] = item;
	((menucommon_s*)menu->items[menu->nitems])->parent        = menu;
	((menucommon_s*)menu->items[menu->nitems])->menuPosition  = menu->nitems;
	((menucommon_s*)menu->items[menu->nitems])->flags        &= ~QMF_HASMOUSEFOCUS;

	// perform any item specific initializations
	itemptr = (menucommon_s*)item;
	switch (itemptr->type)
	{
		case MTYPE_ACTION:
			Action_Init((menuaction_s*)item);
			break;

		case MTYPE_LIST:
			ListBox_Init((menulist_s*)item);
			break;

		case MTYPE_FIELD:
			MenuField_Init((menufield_s*)item);
			break;

		case MTYPE_SPINCONTROL:
			SpinControl_Init((menulist_s*)item);
			break;

		case MTYPE_RADIOBUTTON:
			RadioButton_Init((menuradiobutton_s*)item);
			break;

		case MTYPE_SLIDER:
			Slider_Init((menuslider_s*)item);
			break;

		case MTYPE_BITMAP:
			Bitmap_Init((menubitmap_s*)item);
			break;

		case MTYPE_TEXT:
			Text_Init((menutext_s*)item);
			break;

		case MTYPE_SCROLLLIST:
			ScrollList_Init((menulist_s*)item);
			break;
	}

	menu->nitems++;
}

/*
===============
Menu_CursorMoved
===============
*/
void Menu_CursorMoved( menuframework_s *m )
{
	void (*callback)( void *self, int notification );
	
	if (m->cursor_prev == m->cursor)
		return;

	if (m->cursor_prev >= 0 && m->cursor_prev < m->nitems)
	{
		callback = ((menucommon_s*)(m->items[m->cursor_prev]))->callback;
		if (callback)
			callback(m->items[m->cursor_prev],QM_LOSTFOCUS);
	}
	
	if (m->cursor >= 0 && m->cursor < m->nitems)
	{
		callback = ((menucommon_s*)(m->items[m->cursor]))->callback;
		if (callback)
			callback(m->items[m->cursor],QM_GOTFOCUS);
	}
}



/*
===============
Menu_SetCursor
===============
*/
void Menu_SetCursor( menuframework_s *m, int cursor )
{
	m->cursor_prev = m->cursor;
	m->cursor      = cursor;

	Menu_CursorMoved( m );
}

/*
** Menu_AdjustCursor
**
** This function takes the given menu, the direction, and attempts
** to adjust the menu's cursor so that it's at the next available
** slot.
*/
void Menu_AdjustCursor( menuframework_s *m, int dir ) {
	menucommon_s	*item = NULL;
	qboolean		wrapped = qfalse;

wrap:
	while ( m->cursor >= 0 && m->cursor < m->nitems ) {
		item = ( menucommon_s * ) m->items[m->cursor];
		if ( item->type == MTYPE_SEPARATOR || ( item->flags & (QMF_GRAYED|QMF_HIDDEN|QMF_MOUSEONLY|QMF_INACTIVE) ) ) {
			m->cursor += dir;
		}
		else {
			break;
		}
	}

	if ( dir == 1 ) {
		if ( m->cursor >= m->nitems ) {
			if ( m->wrapAround ) {
				if ( wrapped ) {
					m->cursor = m->cursor_prev;
					return;
				}
				m->cursor = 0;
				wrapped = qtrue;
				goto wrap;
			}
			m->cursor = m->cursor_prev;
		}
	}
	else {
		if ( m->cursor < 0 ) {
			if ( m->wrapAround ) {
				if ( wrapped ) {
					m->cursor = m->cursor_prev;
					return;
				}
				m->cursor = m->nitems - 1;
				wrapped = qtrue;
				goto wrap;
			}
			m->cursor = m->cursor_prev;
		}
	}
}

/*
===============
Menu_Center
===============
*/
void Menu_Center( menuframework_s *menu )
{
	int height;

	height = ( ( menucommon_s * ) menu->items[menu->nitems-1])->y;
	height += 10;
}

/*
===============
Menu_DrawStatusBar
===============
*/
void Menu_DrawStatusBar( const char *string )
{
	if ( string )
		UI_DrawProportionalString( SCREEN_WIDTH/2, SCREEN_HEIGHT - 17, string, UI_CENTER|UI_BIGFONT, menu_dark_color );
}

/*
===============
Menu_Draw
===============
*/
void Menu_Draw( menuframework_s *menu )
{
	int i;
	menucommon_s	*itemptr;

	if ( menu->title )
	{
		UI_DrawProportionalString(SCREEN_WIDTH/2, 0, menu->title, UI_CENTER|UI_UNDERLINE|UI_BIGFONT, menu_text_color );
	}

	/*
	** draw contents
	*/
	for ( i = 0; i < menu->nitems; i++ )
	{
		itemptr = (menucommon_s*)menu->items[i];
		if (itemptr->flags & QMF_HIDDEN)
			continue;

		if (itemptr->ownerdraw)
		{
			// total subclassing, owner draws everything
			itemptr->ownerdraw( itemptr );
		}	
		else 
		{
			switch (itemptr->type)
			{	
				case MTYPE_RADIOBUTTON:
					RadioButton_Draw( (menuradiobutton_s*)itemptr );
					break;

				case MTYPE_FIELD:
					MenuField_Draw( (menufield_s*)itemptr );
					break;
		
				case MTYPE_SLIDER:
					Slider_Draw( (menuslider_s*)itemptr );
					break;
 
				case MTYPE_LIST:
					ListBox_Draw( (menulist_s*)itemptr );
					break;

				case MTYPE_SPINCONTROL:
					SpinControl_Draw( (menulist_s*)itemptr );
					break;
		
				case MTYPE_ACTION:
					Action_Draw( (menuaction_s*)itemptr );
					break;
		
				case MTYPE_SEPARATOR:
					Separator_Draw( (menuseparator_s*)itemptr );
					break;

				case MTYPE_BITMAP:
					Bitmap_Draw( menu,i);
					break;

				case MTYPE_TEXT:
					Text_Draw( menu,(menutext_s*)itemptr );
					break;

				case MTYPE_SCROLLLIST:
					ScrollList_Draw( (menulist_s*)itemptr );
					break;
			}
		}
#ifndef NDEBUG
		if( uis.debug ) {
			int	x;
			int	y;
			int	w;
			int	h;

			if( !( itemptr->flags & QMF_INACTIVE ) ) {
				x = itemptr->left;
				y = itemptr->top;
				w = itemptr->right - itemptr->left + 1;
				h =	itemptr->bottom - itemptr->top + 1;

				if (itemptr->flags & QMF_HASMOUSEFOCUS) {
					UI_DrawRect(x, y, w, h, colorTable[CT_WHITE] );
				}
				else {
					UI_DrawRect(x, y, w, h, colorTable[CT_WHITE] );
				}
			}
		}
#endif
	}

	itemptr = (menucommon_s *) Menu_ItemAtCursor( menu );
	if ( itemptr )
	{
		if ( itemptr->statusbarfunc )
			itemptr->statusbarfunc( ( void * ) itemptr );
		else if ( itemptr->statusbar )
			Menu_DrawStatusBar( itemptr->statusbar );
		else
			Menu_DrawStatusBar( menu->statusbar );
	}
	else
	{
		Menu_DrawStatusBar( menu->statusbar );
	}
}

/*
=================
Menu_ItemAtCursor
=================
*/
void *Menu_ItemAtCursor( menuframework_s *m )
{
	if ( m->cursor < 0 || m->cursor >= m->nitems )
		return 0;

	return m->items[m->cursor];
}

/*
=================
Menu_ActivateItem
=================
*/
sfxHandle_t Menu_ActivateItem( menuframework_s *s, menucommon_s* item ) {
	if ( item->callback ) {
		item->callback( item, QM_ACTIVATED );
//		if( !( item->flags & QMF_SILENT ) ) 
		{
			return menu_move_sound;
		}
	}
	return 0;
}

/*
===============
Menu_SetStatusBar
===============
*/
void Menu_SetStatusBar( menuframework_s *m, const char *string )
{
	m->statusbar = string;
}

/*
===============
Menu_DefaultKey
===============
*/
sfxHandle_t Menu_DefaultKey( menuframework_s *m, int key )
{
	sfxHandle_t		sound = 0;
	menucommon_s	*item;
	int				cursor_prev;

	// menu system keys
	switch ( key )
	{
		case K_MOUSE2:
		case K_ESCAPE:
			if (uis.menusp>0)
			{
				UI_PopMenu();
				return menu_out_sound;
			}
			else	// You're on main menu - no where to back up to.
			{
				return (0);
			}
	}

	if (!m || !m->nitems)
		return (0);

	// route key stimulus to widget
	item = (menucommon_s *) Menu_ItemAtCursor( m );
	if (item && !(item->flags & (QMF_GRAYED|QMF_HIDDEN|QMF_MOUSEONLY|QMF_INACTIVE)))
	{
		switch (item->type)
		{
			case MTYPE_LIST:
				sound = ListBox_Key( (menulist_s*)item, key );
				break;

			case MTYPE_SPINCONTROL:
				sound = SpinControl_Key( (menulist_s*)item, key );
				break;

			case MTYPE_RADIOBUTTON:
				sound = RadioButton_Key( (menuradiobutton_s*)item, key );
				break;

			case MTYPE_SLIDER:
				sound = Slider_Key( (menuslider_s*)item, key );
				break;

			case MTYPE_SCROLLLIST:
				sound = ScrollList_Key( (menulist_s*)item, key );
				break;

			case MTYPE_FIELD:
				switch ( key ) {
				case K_KP_ENTER:
				case K_ENTER:
				case K_JOY1:
				case K_JOY2:
				case K_JOY3:
				case K_JOY4:
					key = K_TAB;		// have enter go to next cursor point
					break;

				case K_TAB:
				case K_KP_DOWNARROW:
				case K_DOWNARROW:
				case K_KP_UPARROW:
				case K_UPARROW:
					break;

				default:
					if ( key & K_CHAR_FLAG ) {
						Field_CharEvent( &(( menufield_s * )item)->field, key & ~K_CHAR_FLAG );
					} else {
						Field_KeyDownEvent( &(( menufield_s * )item)->field, key );
					}
					return NULL;
				}
		}

		if (sound) {
			// key was handled
			return sound;
		}
	}

	// default handling
	switch ( key )
	{
#ifndef NDEBUG
		case K_F11:
			uis.debug ^= 1;
			break;

		case K_F12:
			ui.Cmd_ExecuteText(EXEC_APPEND, "screenshot");
			break;
#endif
		case K_KP_UPARROW:
		case K_UPARROW:
			cursor_prev    = m->cursor;
			m->cursor_prev = m->cursor;
			m->cursor--;
			Menu_AdjustCursor( m, -1 );
			if ( cursor_prev != m->cursor ) {
				Menu_CursorMoved( m );
				sound = menu_move_sound;
			}
			break;

		case K_TAB:
		case K_KP_DOWNARROW:
		case K_DOWNARROW:
			cursor_prev    = m->cursor;
			m->cursor_prev = m->cursor;
			m->cursor++;
			Menu_AdjustCursor( m, 1 );
			if ( cursor_prev != m->cursor ) {
				Menu_CursorMoved( m );
				sound = menu_move_sound;
			}
			break;

		case K_MOUSE1:
		case K_MOUSE3:
			if (item)
				if ((item->flags & QMF_HASMOUSEFOCUS) && !(item->flags & (QMF_GRAYED|QMF_INACTIVE)))
					return (Menu_ActivateItem( m, item ));
			break;

		case K_JOY1:
		case K_JOY2:
		case K_JOY3:
		case K_JOY4:
		case K_AUX1:
		case K_AUX2:
		case K_AUX3:
		case K_AUX4:
		case K_AUX5:
		case K_AUX6:
		case K_AUX7:
		case K_AUX8:
		case K_AUX9:
		case K_AUX10:
		case K_AUX11:
		case K_AUX12:
		case K_AUX13:
		case K_AUX14:
		case K_AUX15:
		case K_AUX16:
		case K_KP_ENTER:
		case K_ENTER:
			if (item)
				if (!(item->flags & (QMF_MOUSEONLY|QMF_GRAYED|QMF_INACTIVE)))
					return (Menu_ActivateItem( m, item ));
			break;
	}

	return sound;
}

/*
===============
Menu_Focus
===============
*/
void Menu_Focus( menucommon_s *m )
{
	// route focus to handler
	switch (m->type)
	{
		case MTYPE_LIST:
			ListBox_Focus( (menulist_s*)m );
			break;
	}	
}

/*
=================
Menu_Cache
=================
*/
void Menu_Cache( void )
{
	int i;
	const char *smallNumbers[]= 
	{
	"gfx/2d/numbers/s_zero.tga",
	"gfx/2d/numbers/s_one.tga",
	"gfx/2d/numbers/s_two.tga",
	"gfx/2d/numbers/s_three.tga",
	"gfx/2d/numbers/s_four.tga",
	"gfx/2d/numbers/s_five.tga",
	"gfx/2d/numbers/s_six.tga",
	"gfx/2d/numbers/s_seven.tga",
	"gfx/2d/numbers/s_eight.tga",
	"gfx/2d/numbers/s_nine.tga",
	};

	//fonts
//	uis.charset		= ui.R_RegisterShaderNoMip( "gfx/2d/trekchars" );
	uis.charset		= ui.R_RegisterShaderNoMip( "gfx/2d/charsgrid_med");
//	uis.cursor		= ui.R_RegisterShaderNoMip( "menu/arena/3_cursor2");
	uis.cursor		= ui.R_RegisterShaderNoMip( "menu/common/cursor");
	uis.charsetPropTiny = ui.R_RegisterShaderNoMip("gfx/2d/chars_tiny");
	uis.charsetProp = ui.R_RegisterShaderNoMip("gfx/2d/chars_medium");
	uis.charsetPropBig = ui.R_RegisterShaderNoMip("gfx/2d/chars_big");

	// Common menu graphics
	uis.graphicCircle = ui.R_RegisterShaderNoMip("menu/common/circle.tga");
	uis.graphicCircle2 = ui.R_RegisterShaderNoMip("menu/objectives/circle.tga");
	uis.graphicEmptyCircle2 = ui.R_RegisterShaderNoMip("menu/objectives/circle_out.tga");
	uis.graphicButtonLeftEnd = ui.R_RegisterShaderNoMip("menu/common/barbuttonleft.tga");

	uis.graphicBracket1CornerLU =  ui.R_RegisterShaderNoMip("menu/common/corner_lu.tga");

	for (i=0;i<10;++i)
	{
		uis.smallNumbers[i] =  ui.R_RegisterShaderNoMip(smallNumbers[i]);
	}

	uis.whiteShader = ui.R_RegisterShader( "white" );
	uis.menuBackShader = ui.R_RegisterShader( "gfx/2d/conback.tga" );
	// When hitting enter
	menu_in_sound	= ui.S_RegisterSound( "sound/interface/button2.wav" );//"sound/interface/jules/mp3");//"sound/interface/button2.wav" );
	// When dragging mouse over buttons
	menu_move_sound	= ui.S_RegisterSound( "sound/interface/button1.wav" );//"sound/interface/jules.mp3" );//"sound/interface/button1.wav" );
	uis.menu_choice1_snd	= ui.S_RegisterSound( "sound/interface/button3.wav" );
	uis.menu_datadisp1_snd	= ui.S_RegisterSound( "sound/interface/button4.wav" );
	uis.menu_datadisp2_snd	= ui.S_RegisterSound( "sound/interface/button7.wav" );
	uis.menu_datadisp3_snd	= ui.S_RegisterSound( "sound/interface/button5.wav" );

	menu_out_sound	= ui.S_RegisterSound( "sound/interface/button2.wav" );
	menu_buzz_sound	= ui.S_RegisterSound( "sound/interface/button2.wav" );

	// need a nonzero sound, make an empty sound for this
	menu_null_sound = -1;

	ui.R_RegisterShaderNoMip(GRAPHIC_BUTTONRIGHT);
	ui.R_RegisterShaderNoMip(GRAPHIC_BUTTONSLIDER);
}

/*
=================
Mouse_Hide
=================
*/
void Mouse_Hide(void)
{
	uis.cursorDraw = qfalse;
}

/*
=================
Mouse_Show
=================
*/
void Mouse_Show(void)
{
	uis.cursorDraw = qtrue;
}
