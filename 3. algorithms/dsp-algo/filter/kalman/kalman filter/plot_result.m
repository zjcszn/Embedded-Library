%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% FileName : plot_result.m
% Author   : xiahouzuoxin @163.com
% Version  : v1.0
% Date     : 2014/9/25 9:13:09
% Brief    : 
% 
% Copyright (C) MICL,USTB
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

clear all
clc;
close all;

[fname, fpath] = uigetfile(...
    {'*.txt', '*.*'}, ...
    'Pick a file');

x = load(fullfile(fpath, fname));

% ��C�����еĴ�ӡ��ʽ����
%     data = x1;
%     data_len = sizeof(x1)/sizeof(float);
%     kalman1_init(&state, data[0], 5e2);
%     printf("%d %d\n", data, data_len-1);
%     for (i=1; i<data_len; i++) {
%         printf("%.2f", data[i]);      // Original data
%         data[i] = kalman1_filter(&state, data[i]);
%         printf(" %.2f\n", data[i]);  // Filter result
%     }
% 
%     data = x2;
%     data_len = sizeof(x2)/sizeof(float);
%     printf("%d %d\n", data, data_len-1);
%     kalman1_init(&state, data[0], 5e2);
%     for (i=1; i<data_len; i++) {
%         printf("%.2f", data[i]);      // Original data
%         data[i] = kalman1_filter(&state, data[i]);
%         printf(" %.2f\n", data[i]);  // Filter result
%     }

x1_start = 2;
x1_end   = 1 + x(1,2);
x1 = x(x1_start:x1_end, :);

x2_start = x1_end + 2;
x2 = x(x2_start+1:end, :);

figure,

subplot(211);
H1 = plot(x1(:,1), 'r.');
hold on;
H2 = plot(x1(:,2), 'b-');
hold on;
H3 = plot(x1(:,3), '-', 'Color',[3 3 3]./255, 'LineWidth',2);
legend([H1, H2, H3], 'ԭ����', 'һάKalman Filter', '��άKalman Filter');
grid on

subplot(212);
H1 = plot(x2(:,1), 'r.');
hold on;
H2 = plot(x2(:,2), 'b-');
hold on;
H3 = plot(x2(:,3), '-', 'Color',[3 3 3]./255, 'LineWidth',2);
legend([H1, H2, H3], 'ԭ����', 'һάKalman Filter', '��άKalman Filter');
grid on


