function[dataSet,dataSet_means ] = loadData( str );
    dataTable = readtable(str);
    
    data_1 = table2array(dataTable(1:10,2:11));
    data_2 = table2array(dataTable(12:21,2:11));
    data_3 = table2array(dataTable(23:32,2:11));
    data_4 = table2array(dataTable(34:43,2:11));
    data_5 = table2array(dataTable(45:54,2:11));
    
    dataSet = zeros(10,10,5);
    dataSet(:,:,1)=data_1;
    dataSet(:,:,2)=data_2;
    dataSet(:,:,3)=data_3;
    dataSet(:,:,4)=data_4;
    dataSet(:,:,5)=data_5;

dataSet_means = [mean(dataSet(:,10,1)),mean(dataSet(:,10,2)),mean(dataSet(:,10,3)),mean(dataSet(:,10,4)),mean(dataSet(:,10,5)),];


end
% dataSet(:,10,1)


% x=[10*ones(10,1);20*ones(10,1);30*ones(10,1);40*ones(10,1);50*ones(10,1)];
% z=[dataSet(:,10,1)';dataSet(:,10,2)';dataSet(:,10,3)';dataSet(:,10,4)';dataSet(:,10,5)']';
% y=[1:10,1:10,1:10,1:10,1:10]';
% figure
% bar3([1:10]',z)
% set(gca,'XTick',1:5)
% set(gca,'XTickLabel',{'w10','w20','w100','w200','w500'})
% xlabel("window size")
% zlabel("total throughput")
% xlabel()
% set(gca,'YTick',1:4)
% set(gca,'YTickLabel',{'江南','塞北'})

% scatter3(x,y,z)
%读取x、y值
