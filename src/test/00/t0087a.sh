#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1995, 1997, 1998, 2001, 2007 Peter Miller;
#       All rights reserved.
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 3 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program. If not, see
#       <http://www.gnu.org/licenses/>.
#
work=${COOK_TMP:-/tmp}/$$
PAGER=cat
export PAGER
umask 022
unset COOK
here=`pwd`
if test $? -ne 0 ; then exit 1; fi

bin="$here/${1-.}/bin"

fail()
{
        set +x
        echo 'FAILED test of the data dataend functionality' 1>&2
        cd $here
        rm -rf $work
        exit 1
}
pass()
{
        set +x
        cd $here
        rm -rf $work
        exit 0
}
trap \"fail\" 1 2 3 15

mkdir $work $work/lib
if test $? -ne 0 ; then exit 1; fi
cd $work
if test $? -ne 0 ; then fail; fi

#
# Use the default error messages.  There is no other way to get
# predictable test behaviour on the unknown systems we will be tested on.
#
COOK_MESSAGE_LIBRARY=$work/no-such-dir
export COOK_MESSAGE_LIBRARY
unset LANG

#
# test the data dataend functionality
#
cat > Howto.cook << 'fubar'
test:
{
        cat > [target];
        data
478419787 1848587843 366432946 1415719421 685962344 800787207 512887343
2103883632 1553007812 907276668 939183245 2033091504 1015482697 498644539
308421973 1201282452 125204477 1443054657 378522298 840169048 306581513
424001999 1203992315 313286408 433117372 1800006281 172837093 1388738078
1499419515 2144306038 1664317372 1977839302 1845410233 2030750318 1246075075
383888930 684053877 1758962419 340288914 89578041 518755439 1279472160
2122669546 1534238137 1778116699 283607871 588036941 1903321176 1726662529
966559240 596006576 2033244042 1390561239 1799998891 199046802 1823678611
1452521525 371883895 1064933041 804457392 368706286 581766765 634813046
66632871 465033435 1880888121 450521801 1149087313 1492366892 790810716
1238665354 2011122332 2070282876 1213851252 1397876821 1700915927 1497459124
1985913762 1456753455 1076638005 804989354 2052760031 962398399 48066946
1705275274 1161445202 1871745557 1010313151 1533329097 789194951 1814770543
1902035383 1370961716 302099941 1968668255 1835995152 35504415 271706408
837598817 1527871307 1062517124 2076264171 1391509991 985316352 1142631776
641903164 538748631 492607252 480333279 1995502086 1569245257 1285322633
1900778469 384160008 1333389579 1458570096 1545605210 1057651489 321399599
931450660 1846846440 2136170143 686002395 1070324508 290786436 507187002
758836012 326290851 778893411 1596434829 1854162159 1841410535 1525215353
1098188502 679243240 520363481 1740091667 1217991871 1012970733 72941298
1066010310 434732342 1358263931 819305131 818892350 544169863 130391579
217013913 1601821352 451791179 1148464573 1301184144 440477674 1834466968
224025004 731264110 194170323 982861017 1057554962 973063734 431812198
764233473 666990621 1957027551 1862421975 1346233861 329907384 1455029994
416742085 1342878117 1527971292 1482752395 1777610459 738751576 154573878
449019162 1282921439 284965458 666033075 737259143 736756637 1814497648
2038443287 1177234311 1501480968 114984643 1908498421 1695651291 1097845660
818569735 521231377 1529657859 1582803208 1188221999 1339201762 1297741536
386972212 1669109147 605287882 803714297 864503616 2133259175 138983044
494630428 724527103 293556923 943649590 2007448542 578522381 1609682665
597224037 1315279018 1276696665 488183676 345029681 630693985 603168319
106044454 178861629 1701013980 924614190 700093006 1083188191 359933750
1888315005 274906305 1657675286 127803570 1944015452 115479521 931517867
661035421 101255048 1070500912 1155665849 825782151 1364057835 2099315439
685747045 1942580216 1561514456 1282971082 1110375586 690727473 1771154758
1455405267 1321421458 226839429 1561449721 1500283087 1927853409 338580263
52892446 863557952 698514014 1941207451 1138464258 208705652 2069011021
934996062 324185173 853045241 1596031483 425440221 1923546153 604213684
1251222372 1140120340 556045475 1936969417 935216908 2117559931 1072456851
2045592494 660803756 696127961 1353514113 1982225215 922967391 767480186
1335024654 703337152 1106060450 1387917100 1566895105 1804574464 1181640904
557875715 2013280116 1103168277 1492871777 189981642 1956213518 941419613
615421863 1732276023 1545633297 1866644236 724912715 2101678773 1656130005
1660129623 2071755056 581103209 1558238469 585075165 1277231170 764268934
419816732 52714913 1531749121 1754841386 756052066 490325923 995274839
175463523 147416739 29432095 733339238 13213207 1132600372 78727367
203194849 941330243 1020146980 818616713 526122618 418296630 537777301
1251035334 372491755 46423658 763681309 296763163 627526867 174436131
881838328 1904758038 938705065 1301655060 1957472951 322970538 909012799
566041369 813296461 1904287638 741504892 960713200 1933719733 1474844130
973926408 918836457 1553571498 1177121257 1860166700 426234830 1995737970
238805671 844531460 386031623 1489841005 1217023215 432455282 106038666
1513786379 1059982149 280474797 248141059 817256539 1219179863 1549796120
627245843 1542150401 311325271 1193287212 207963215 68129261 1934792105
1168676415 2001848994 1262152587 2142602823 773201803 668240437 1172240433
485884856 1094475268 1020494755 724690527 1939006728 1406526379 67047884
1008546296 1838981661 173086550 374849027 751480162 453561348 622990086
1568736702 1672741211 25302558 48498897 1067407964 336627829 1241786109
1275371179 404757090 1029094566 296563947 259122436 143763506 291683122
1032324240 812003943 1463923555 1518209096 1906479211 336934663 95415975
1698002292 1743461042 162463859 559064940 1434959055 335550409 933913967
38955569 789111757 1556904053 1607692271 314369320 1582206612 1656191168
1381777285 1918834441 750493630 509664816 176107884 1779588196 806228763
435230320 1923351702 1097911886 1467554560 587871998 414351793 838280008
346867561 751286456 933695983 2044869853 347263850 1096159842 456451145
1782222905 1431710252 1390365112 1821178475 73338361 799785518 1281387098
387707682 234508482 790094619 1769484967 5859275 1540588249 131666135
181967159 1172692797 937894899 617197480 948560852 2035806785 2084752040
1536432850 302674930 775548401 1883300411 1053961387 1709244384 1780686617
1401225237 657920579 89654114 1035964495 2089630831 1480019227 709659322
15485544 132321097 1991046420 403193226 366829579 633657391 25194545
372688854 26761992 156860681 554656014 1199454790 1094755580 1171853494
531994 983078717 1109121886 1536964844 1285753647 1884670287 1272781607
192231386 1446431024 905984576 1593456624 2104351603 995638691 481937471
2046498786 328174270 1191596793 2061984330 460495367 1035159565 317693909
827324946 1668816957 342888454 1200013800 1695578949 499749135 1754669814
747550091 1594504715 779039660 748082085 430099784 1888161547 137563281
1715853432 1625348186 1410344889 1908084818 924295562 168845817 1354057794
881163517 1164484508 1835995265 780178655 1492658778 880108410 694679338
1953154145 1915267976 1012373247 632995443 1436601285 1355261701 1833009244
984696586 1855010837 1440195410 1732246678 1302031904 71751423 332845115
1732131689 1959912970 470408397 1300501473 1437777508 1880753286 1061102643
214589423 2049599103 267676790 1095752940 1066599964 2103672055 1875931596
411775094 836296818 423127286 217445592 604081146 1435500533 850441035
2040682431 643278586 535966631 877895369 350805775 1976162042 462658399
1652837680 2047913465 795503515 1237485721 1860342787 1265911912 390503546
1150636647 999181550 1451606189 1365226070 901297005 1719282979 313495363
1967896969 1675471387 41943311 232188416 364284557 465070597 449634008
968365703 1900571130 1300075043 861564486 396366068 1836041675 1739459855
747171844 1664720069 54634607 252525876 1565149886 850138122 1490011597
1278009025 2116050034 1880515143 281162024 967747936 1184637684 1646388095
1869044941 756437016 1959883458 1689458263 284424755 2001826769 1921646679
648709312 319413718 223797039 1617075015 72501200 1523872082 331155853
468867268 1212430109 2070615708 1216039112 729666530 2125250315 1468564988
147332768 827904789 811092937 1425341793 796471175 544124432 1706503818
1764219111 1728762117 1205408265 1485780405 337715485 1017808075 1027755020
622140240 872151196 801918051 1270849552 1191564914 1025715090 740440919
1264066114 402103524 1071596772 1732933382 1614533634 994728832 801488847
196716516 972495500 122570187 344049285 1800400289 933663125 1769391078
449387817 1477787557 1328411248 66123280 1059066026 386335865 1551903685
1396781511 1404143940 432175057 2018921751 128811488 1234093108 1142287655
1320376402 112324550 1882728574 436958868 514428075 806841698 22408603
2128961709 1801570531 823897450 178194577 626582383 946467637 522243862
279499024 1880130762 144151293 728886841 1210434672 1472562541 795010122
122017050 1858898407 199430159 1518798562 1115558699 631605217 1390236665
1244370188 1865698325 385040673 417262942 1978022876 120285599 854221811
344967303 927127298 876630414 326445364 581214181 1700527864 504639941
1207796564 499511853 1026883804 1487295588 232158968 1171035097 68698782
1442593640 496113990 863708904 1564610690 207528749 1063139063 935925604
1323087449 1694744280 178678622 419973989 1412958958 563719295 837236931
1243498186 684004894 1691458742 1588465489 1611132192 420605508 1914910853
44862725 2121133372 272067146 1252659289 473161578 1298950950 592471230
705320546 322502399 661170012 430538 818616390 1524878916 1565041228
1026145139 440534331 353483185 201748940 2135278612 532161807 621722929
1400753922 1095881102 1458959861 496768460 1779885996 1002934955 2085233949
1243534541 1423540464 1852661154 1288397266 1397190188 2124728300 393572908
1870351766 1276195603 986044138 428188664 1598698002 1647214150 428619202
269830744 1024609418 1993660431 1295975884 1465143749 199659968 1497724824
1452938713 731821775 2119447754 706208987 1827702877 1430923967 1202977447
1460105225 286375274 1140727748 556156118 1709915738 845905254 1844553385
959622279 823149907 90642645 682490397 2099345510 1076686783 1110679062
1550559864 576417285 1539298264 1820390609 1601026703 1385475047 968882845
918686804 1585135015 319124021 224141870 169473142 291088127 930350857
1997176019 1722012094 2133328305 1309797597 2008387369 1126572405 1865953715
1570819459 1972477660 1563023452 382958090 648143919 1653666097 1065448488
600005781 582869232 28643902 3081997 1159286517 1567942166 1823472606
612829572 805933566 644871803 1531516377 243584933 963995825 1755658247
413058076 1255083952 538525456 262750447 829612399 524370113 1572548044
690516120 1650942519 1291018112 113851931 1475936531 706557916 496810022
2124080450 212740366 1562258510 576602583 795609598 1590902412 579684580
1954896116 1011360930 255673539 420242040 1817294496 900545342 1951758417
2060879430 1864541167 1559933016 326453858 972141472 2098458473 589204305
1801753871 475344938 14268702 344786343 2126287457 1305286814 458638274
1454740340 2011844730 955448296 1431337142 77101448 370223158 2007939725
872711047 1961125570 440140658 680123515 825002853 695814197 1100365555
494813701 1596359539 904640325 408209483 1313417059 317089693 734663341
138074883 268064518 1323867647 1939828754 743409457 1338136349 137131449
722213266 495939515 595769723 29469959 360300597 1551218020 1460807101
437402046 1921441178 1321263179 1310113093 1735083101 1761403837 1990236608
412602306 309734386 943118515 907416007 1906093925 1847758840 1315625491
1072027336 17364886 2050288832 1210102219 285429404 1226672831 1002447325
1028838861 417325532 1139578774 1751052128 913265047 1735348498 1780522087
1273565645 1139082870 1093845540 1710967691 913040400 267625071
        dataend
}
fubar
if test $? -ne 0 ; then fail; fi

$bin/cook -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
if test ! -r test ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass