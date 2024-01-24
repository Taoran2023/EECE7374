clc
clear
close all

[sr_08,sr_08_means] = loadData("sr_08.csv");
[gbn_08,gbn_08_means] = loadData("gbn_08.csv");
[abt,abt_means] = loadData("abt.csv");

abt_08 = abt_means(5)*ones(1,5);

[sr_02,sr_02_means] = loadData("sr_02.csv");
[gbn_02,gbn_02_means] = loadData("gbn_02.csv");

abt_02 = abt_means(2)*ones(1,5);


[sr_05,sr_05_means] = loadData("sr_05.csv");
[gbn_05,gbn_05_means] = loadData("gbn_05.csv");

AbtdataTable = readtable("abt.csv");
abtdata_5 = table2array(AbtdataTable(56:65,2:11));
abt_05 = mean(abtdata_5(:,10))*ones(1,5);

ThroughPut_02 = [abt_02;gbn_02_means;sr_02_means];
ThroughPut_05 = [abt_05;gbn_05_means;sr_05_means];
ThroughPut_08 = [abt_08;gbn_08_means;sr_08_means];

x= [1:5];

figure
bar(x,ThroughPut_02)
title("ThroughPut VS WindowSize(Loss Rate 0.2)")
legend("abt","gbn","sr")
xlabel("Window Size")
ylabel("Average Throughput")
set(gca,'XTickLabel',{'10','50','100','200','500'})
grid on

figure
bar(x,ThroughPut_05)
title("ThroughPut VS WindowSize(Loss Rate 0.5)")
legend("abt","gbn","sr")
xlabel("Window Size")
ylabel("Average Throughput")
set(gca,'XTickLabel',{'10','50','100','200','500'})
grid on

figure
bar(x,ThroughPut_08)
title("ThroughPut VS WindowSize(Loss Rate 0.8)")
legend("abt","gbn","sr")
xlabel("Window Size")
ylabel("Average Throughput")
axis([0.5111 5.4889 0 0.0250])
grid on

set(gca,'XTickLabel',{'10','50','100','200','500'})
