#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

circ = [0.446798,-0.843918,-0.961892,-0.965030,-0.957982,-0.970353,-0.967576,-0.963139,-0.959382,-0.972216,-0.971981,-0.974026,-0.952486,-0.962896,-0.958565,-0.966111,-0.977506,-0.967581,-0.966436,-0.957789,-0.952892,-0.950160,-0.953762,-0.970926,-0.972742,-0.966118,-0.972599,-0.966946,-0.963507,-0.957845,-0.966075,-0.969638,-0.972992,-0.973917,-0.972713,-0.973092,-0.971381,-0.964692,-0.970766,-0.972259,-0.972079,-0.963701,-0.971360,-0.954272,-0.967989,-0.950873,-0.970571,-0.967228,-0.958467,-0.940491,-0.938323,-0.946711,-0.887604,-0.367264,0.869880,0.921147,0.864230,0.999182,0.690176,0.915346,1.000000,0.855273,0.898162,0.384069,0.557744,0.636268,0.994524,0.869004,0.706946,0.804893,0.370795,0.962644,0.693426,0.450432,0.888348,0.450838,0.302843,0.592254,0.711250,0.789378,0.555612,0.079402,0.844560,0.619340,-0.172232,0.656798,0.476374,0.409030,0.481129,0.657262,0.064926,0.112142,0.262972,0.250466,0.244377,0.196604,0.161233,-0.308538,0.247051,-0.130626,0.093008,-0.464946,0.041207,-0.374536,-0.411710,-0.147483,-0.239240,-0.329720,-0.700077,-0.308827,-0.441625,-0.581791,-0.417666,-0.457080,-0.522767,-0.607681,-0.649263,-0.551173,-0.649502,-0.640525,-0.813994,-0.653590,-0.738870,-0.774591,-0.803711,-0.829572,-0.854921,-0.897746,-0.858004,-0.934902,-0.904941,-0.908574,-0.907431,-0.924668,-0.912767,-0.914308,-0.923922,-0.925705,-0.928507,-0.915904,-0.930693,-0.926199,-0.936141,-0.944262,-0.954124,-0.944186,-0.935262,-0.935053,-0.940848,-0.933166,-0.950420,-0.947461,-0.961763,-0.951992,-0.938184,-0.955397,-0.938542,-0.933014,-0.923556,-0.940224,-0.914930,-0.921246,-0.913325,-0.915248,-0.915347,-0.917751,-0.904243,-0.856710,-0.868615,-0.884707,-0.880563,-0.843090,-0.902597,-0.840006,-0.809371,-0.878491,-0.822253,-0.809129,-0.809231,-0.808002,-0.826396,-0.796421,-0.737415,-0.775215,-0.712809,-0.682694,-0.765103,-0.713749,-0.726636,-0.691739,-0.652214,-0.703500,-0.705772,-0.716106,-0.608455,-0.603880,-0.571825,-0.587674,-0.615764,-0.538761,-0.567340,-0.525049,-0.539201,-0.812996,-0.942504,-0.960646,-0.961698,-0.961958,-0.970529,-0.961714,-0.962053,-0.956463,-0.938691,-0.954986,-0.958873,-0.959778,-0.959232,-0.965450,-0.970472,-0.972666,-0.968570,-0.971376,-0.970359,-0.967171,-0.970863,-0.972482,-0.976475,-0.970215,-0.967334,-0.971539,-0.970403,-0.968460,-0.969949,-0.965904,-0.951145,-0.950589,-0.966245,-0.966596,-0.969365,-0.973206,-0.974537,-0.968495,-0.968012,-0.963130,-0.960465,-0.968465,-0.971450,-0.972136,-0.972194,-0.970493,-0.971016,-0.971684,-0.970222,-0.969660,-0.973432]
star = [0.399278,-0.848655,-0.962715,-0.971071,-0.976936,-0.977805,-0.975485,-0.968776,-0.975336,-0.979236,-0.976645,-0.970059,-0.975541,-0.978094,-0.979670,-0.977389,-0.977365,-0.977531,-0.975775,-0.980350,-0.975689,-0.976690,-0.975257,-0.972442,-0.974142,-0.962014,-0.973073,-0.973253,-0.978483,-0.976907,-0.975848,-0.976710,-0.975653,-0.974576,-0.974165,-0.980355,-0.981217,-0.977706,-0.976999,-0.978747,-0.980034,-0.974120,-0.974744,-0.977741,-0.977359,-0.978087,-0.973769,-0.970661,-0.961789,-0.964173,-0.959554,-0.945645,-0.897343,-0.000036,0.570861,0.876406,0.875657,0.602947,0.560767,0.540522,0.424135,0.911551,0.678738,0.217089,1.000000,0.382124,0.812890,0.250914,0.595204,0.281654,0.408784,0.675970,0.721621,0.583464,0.498782,0.281879,0.482094,0.535366,0.393295,0.703091,0.214774,0.301809,0.393324,0.637646,-0.075345,0.624621,0.005315,0.327186,0.370731,-0.088235,0.112296,0.322480,0.262875,0.011721,0.079705,-0.059960,0.003684,0.010225,-0.037327,0.122000,-0.452599,-0.194905,-0.295761,-0.264611,-0.287026,-0.350241,-0.348147,-0.676773,-0.356070,-0.383689,-0.472999,-0.786728,-0.423071,-0.497180,-0.718231,-0.582478,-0.601712,-0.631848,-0.579080,-0.757044,-0.678983,-0.743153,-0.797925,-0.778529,-0.815229,-0.830860,-0.872422,-0.920335,-0.871019,-0.881352,-0.904680,-0.936055,-0.930586,-0.919692,-0.911164,-0.932394,-0.929015,-0.935554,-0.924713,-0.931232,-0.940999,-0.934627,-0.951123,-0.943341,-0.947777,-0.939673,-0.935271,-0.947111,-0.959710,-0.951050,-0.955213,-0.949720,-0.959858,-0.951844,-0.946532,-0.947640,-0.961261,-0.939297,-0.939988,-0.933603,-0.928412,-0.936209,-0.923353,-0.915110,-0.902125,-0.913998,-0.910684,-0.913215,-0.887810,-0.881873,-0.900777,-0.854003,-0.890535,-0.830391,-0.858310,-0.861441,-0.863134,-0.823809,-0.838439,-0.810246,-0.825092,-0.835524,-0.795106,-0.782660,-0.758436,-0.748624,-0.764477,-0.789280,-0.738068,-0.783481,-0.679313,-0.733831,-0.623059,-0.764932,-0.652675,-0.612528,-0.703216,-0.663107,-0.633723,-0.608113,-0.555416,-0.581228,-0.601590,-0.734126,-0.953401,-0.970569,-0.970945,-0.971565,-0.975191,-0.972344,-0.977543,-0.970240,-0.974091,-0.976011,-0.972299,-0.969837,-0.970354,-0.971987,-0.970497,-0.971746,-0.972532,-0.975946,-0.973516,-0.977766,-0.973813,-0.974267,-0.977989,-0.976520,-0.975125,-0.974441,-0.976357,-0.976441,-0.974254,-0.975894,-0.977461,-0.976121,-0.975945,-0.976950,-0.976457,-0.973588,-0.978617,-0.977839,-0.977967,-0.978215,-0.976762,-0.973728,-0.979551,-0.974797,-0.974650,-0.974308,-0.975392,-0.976581,-0.976316,-0.977969,-0.978669]

freq = np.fft.fftfreq(255*2, 1/2.4e6)[:255] / 1e6

plt.figure()
plt.xlabel("Frequency (GHz)")
plt.ylabel("Relative power")
plt.grid()
plt.plot(freq, circ)

plt.figure()
plt.xlabel("Frequency (GHz)")
plt.ylabel("Relative power")
plt.grid()
plt.plot(freq, star)

plt.show()
