clc
clear
close all

[sr_10,sr_10_means] = loadData("sr_10.csv");
[gbn_10,gbn_10_means] = loadData("gbn_10.csv");
[abt,abt_means] = loadData("abt.csv");

% abt_08 = abt_means(5)*ones(1,5);

[sr_50,sr_50_means] = loadData("sr_50.csv");
[gbn_50,gbn_50_means] = loadData("gbn_50.csv");

% abt_02 = abt_means(2)*ones(1,5);


ThroughPut_10 = [abt_means;gbn_10_means;sr_10_means];
ThroughPut_50 = [abt_means;gbn_50_means;sr_50_means];


x= [1:5];

figure
bar(x,ThroughPut_10)
title("ThroughPut VS Loss Rate(WindowSize 10)")
legend("abt","gbn","sr")
xlabel("Loss Probability")
ylabel("Average Throughput")
set(gca,'XTickLabel',{'0.1','0.2','0.4','0.6','0.8'})
grid on

figure
bar(x,ThroughPut_50)
title("ThroughPut VS Loss Rate(WindowSize 50)")
legend("abt","gbn","sr")
xlabel("Loss Probability")
ylabel("Average Throughput")
set(gca,'XTickLabel',{'0.1','0.2','0.4','0.6','0.8'})
grid on
