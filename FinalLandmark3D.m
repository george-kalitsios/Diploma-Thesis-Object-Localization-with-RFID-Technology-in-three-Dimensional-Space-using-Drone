%Diploma Thesis , June 2020
%George Kalitsios ,THMMY AUTH
%3D_Landmarc_Localization_Algorithm 
%-------------------------------------------------------------------------
%Hyperparameters
k_neighbors=8; 
dt=1;
common_samples_threshold=7;
%-------------------------------------------------------------------------
Reference_tags = readtable('Reference_All.xlsx','ReadVariableNames',true);
List_of_all_tags=table2array(Reference_tags(:,1));
All_tags = readtable('All_tags.xlsx','ReadVariableNames',true);
[m n]=size(List_of_all_tags);
Number_of_occurences=zeros(m,1);
num_of_ref_tags=m;
RSSIMetric=zeros(m);
RSSIMetricSamples=zeros(m);
CommonSamples=zeros(m);
CommonSamplesAchievedThreshold=zeros(m);
Reader_table = readtable('Setup_2_Run_5.txt','ReadVariableNames',true,'Delimiter','comma');
Measurements=Reader_table;
[m n]=size(Measurements);
i=1;
while i<m
    Reader_table{i+1,1}=((Reader_table{i+1,1}-Reader_table{1,1})/1000)+1;
    i=i+1;
end 
Reader_table{1,1}=1;
Measurements=Reader_table;
[m n]=size(Measurements);
i=1;
localization_loop_initial_time=Reader_table{i,1};
localization_loop_dt=600;
while i<m
    C=[Measurements(i,3) Measurements(i,4) ]; 
    C.Samples=1;
    initial_time=Reader_table{i,1};
    current_time=initial_time;
    while (current_time<=(initial_time+dt)&& i<m)
        i=i+1;
        current_time=Reader_table{i,1};
        temp=Reader_table{i,3};
        index=find(contains(C{:,1},Reader_table{i,3}));
        if not(isempty(index))
            C.RSSI(index)=(C.RSSI(index)*C.Samples(index)+Reader_table{i,4})/(C.Samples(index)+1);
            C.Samples(index)=C.Samples(index)+1;
        else
            nrow = size(C,1);
            Temp=[Measurements(i,3) Measurements(i,4) ];
            Temp.Samples=1;
            C=vertcat(C,Temp);
        end
    end
    [m_in n_in]=size(List_of_all_tags);
    temp = vertcat(List_of_all_tags,C{:,1}); % concatenate reference tags and all tags
    List_of_all_tags = (unique(temp(:,1),'stable')); % keep only the first encounter so we have ref tags at the first rows and other tags follow
    [m_final n_final]=size(List_of_all_tags);
    mi_diff=m_final-m_in;
    if mi_diff>0
        temp2=zeros(mi_diff,1);
        temp1=zeros(mi_diff,num_of_ref_tags);
        Number_of_occurences=vertcat(Number_of_occurences,temp2);
        CommonSamples=vertcat(CommonSamples,temp1);
        CommonSamplesAchievedThreshold=vertcat(CommonSamplesAchievedThreshold,temp1);
        RSSIMetric=vertcat(RSSIMetric,temp1);
        RSSIMetricSamples=vertcat(RSSIMetricSamples,temp1);
    end
    nrowC=size(C,1);
    for k=1:nrowC   
        index_epc1=find(contains(List_of_all_tags,C{k,1}));
        Number_of_occurences(index_epc1)=Number_of_occurences(index_epc1)+1; % I have to update the number of occurences of each tag right here(how many times it has been read in this experiment)
        for j=k+1:nrowC
            index_epc2=find(contains(List_of_all_tags,C{j,1}));
            if index_epc1>num_of_ref_tags && index_epc2<=num_of_ref_tags % index_epc1 is unknown index_epc2 is reference
                RSSIMetric(index_epc1,index_epc2)=(RSSIMetric(index_epc1,index_epc2)*RSSIMetricSamples(index_epc1,index_epc2)+(C{k,2}-C{j,2})^2)/(RSSIMetricSamples(index_epc1,index_epc2)+1);
                RSSIMetricSamples(index_epc1,index_epc2)=RSSIMetricSamples(index_epc1,index_epc2)+1;
                CommonSamples(index_epc1,index_epc2)=CommonSamples(index_epc1,index_epc2)+1;
                if CommonSamples(index_epc1,index_epc2)>common_samples_threshold
                    CommonSamplesAchievedThreshold(index_epc1,index_epc2)=1;
                end

            elseif index_epc1<=num_of_ref_tags && index_epc2>num_of_ref_tags % index_epc2 is unknown index_epc1 is reference
                RSSIMetric(index_epc2,index_epc1)=(RSSIMetric(index_epc2,index_epc1)*RSSIMetricSamples(index_epc2,index_epc1)+(C{k,2}-C{j,2})^2)/(RSSIMetricSamples(index_epc2,index_epc1)+1);
                RSSIMetricSamples(index_epc2,index_epc1)=RSSIMetricSamples(index_epc2,index_epc1)+1;
                CommonSamples(index_epc2,index_epc1)=CommonSamples(index_epc2,index_epc1)+1;
                if CommonSamples(index_epc2,index_epc1)>common_samples_threshold
                    CommonSamplesAchievedThreshold(index_epc2,index_epc1)=1;
                end

            elseif index_epc1<=num_of_ref_tags && index_epc2<=num_of_ref_tags % index_epc1 , index_epc2 both reference
                RSSIMetric(index_epc1,index_epc2)=(RSSIMetric(index_epc1,index_epc2)*RSSIMetricSamples(index_epc1,index_epc2)+(C{k,2}-C{j,2})^2)/(RSSIMetricSamples(index_epc1,index_epc2)+1);
                RSSIMetricSamples(index_epc1,index_epc2)=RSSIMetricSamples(index_epc1,index_epc2)+1;
                CommonSamples(index_epc1,index_epc2)=CommonSamples(index_epc1,index_epc2)+1;
                if CommonSamples(index_epc1,index_epc2)>common_samples_threshold
                    CommonSamplesAchievedThreshold(index_epc1,index_epc2)=1;
                end
                RSSIMetric(index_epc2,index_epc1)=(RSSIMetric(index_epc2,index_epc1)*RSSIMetricSamples(index_epc2,index_epc1)+(C{k,2}-C{j,2})^2)/(RSSIMetricSamples(index_epc2,index_epc1)+1);
                RSSIMetricSamples(index_epc2,index_epc1)=RSSIMetricSamples(index_epc2,index_epc1)+1;
                CommonSamples(index_epc2,index_epc1)=CommonSamples(index_epc2,index_epc1)+1;
                if CommonSamples(index_epc2,index_epc1)>common_samples_threshold
                    CommonSamplesAchievedThreshold(index_epc2,index_epc1)=1;
                end
            end
        end
     end
     C=[];
    if (i==m)
        localization_loop_initial_time=current_time;
        RSSIMetric_Local=RSSIMetric;
        Estimations_RSSI=array2table(List_of_all_tags,'VariableNames',{'EPC'});
        nrow = size(Estimations_RSSI,1);
        Estimations_RSSI.X = zeros(nrow, 1);    
        nrow = size(Estimations_RSSI,1);
        Estimations_RSSI.Y = zeros(nrow, 1);    
        nrow = size(Estimations_RSSI,1);
        Estimations_RSSI.Z= zeros(nrow, 1);    
        Estimations_RSSI.Errors=zeros(nrow, 1);
        for giota=1:m_final
            for kapa=1:num_of_ref_tags
                  if (RSSIMetric_Local(giota,kapa)==0 || CommonSamples(giota,kapa)<common_samples_threshold)
                      RSSIMetric_Local(giota,kapa)=1000000000;
                  else
                      if CommonSamplesAchievedThreshold(giota,kapa)
                         RSSIMetric_Local(giota,kapa)=RSSIMetric_Local(giota,kapa)/CommonSamples(giota,kapa);
                      end
                  end
           end
        end
        for giota=49:m_final
            [out_RSSI,idx_RSSI] = sort(RSSIMetric_Local(giota,:));
            if out_RSSI(1)<1.0000e+09
                Sum_w_RSSI=0;
                for jei=1:k_neighbors
                    Sum_w_RSSI=Sum_w_RSSI+1/(RSSIMetric_Local(giota,idx_RSSI(jei))); 
                    Epc_ref=List_of_all_tags(idx_RSSI(jei));
                    index_of_ref=find(contains(Reference_tags.EPC_TAG,Epc_ref));
                    Estimations_RSSI.X(giota)= Estimations_RSSI.X(giota)+ 1/(RSSIMetric_Local(giota,idx_RSSI(jei)))*Reference_tags.X(index_of_ref);
                    Estimations_RSSI.Y(giota)= Estimations_RSSI.Y(giota)+ 1/(RSSIMetric_Local(giota,idx_RSSI(jei)))*Reference_tags.Y(index_of_ref);
                    Estimations_RSSI.Z(giota)= Estimations_RSSI.Z(giota)+ 1/(RSSIMetric_Local(giota,idx_RSSI(jei)))*Reference_tags.Z(index_of_ref);
                end
                Estimations_RSSI.X(giota)= Estimations_RSSI.X(giota)/Sum_w_RSSI;
                Estimations_RSSI.Y(giota)= Estimations_RSSI.Y(giota)/Sum_w_RSSI;
                Estimations_RSSI.Z(giota)= Estimations_RSSI.Z(giota)/Sum_w_RSSI;
                index_EPC_all_tags=find(contains(All_tags.EPC_TAG(:),Estimations_RSSI.EPC(giota)));
                if isempty(index_EPC_all_tags)==0 
                        Estimations_RSSI.Errors(giota)=sqrt((Estimations_RSSI.X(giota)-All_tags.X(index_EPC_all_tags))^2+(Estimations_RSSI.Y(giota)-All_tags.Y(index_EPC_all_tags))^2+(Estimations_RSSI.Z(giota)-All_tags.Z(index_EPC_all_tags))^2);
                end
            end 
        end
     end
end
fileID2 = fopen('Estimation_results_RSSI_Fingerprinting.txt','w');
fprintf(fileID2,'EPC_TAG\t X\t Y\t Z\t Number_of_occurences\t \n');
for giota=1:m_final
    if (sum(CommonSamplesAchievedThreshold(giota,:))>k_neighbors)
        fprintf(fileID2,'%s\t %f\t %f\t %f\t %d\n',Estimations_RSSI.EPC{giota},Estimations_RSSI.X(giota),Estimations_RSSI.Y(giota),Estimations_RSSI.Z(giota),Number_of_occurences(giota,1));
    end
end
fclose(fileID2);
num=0;
s=0;
for k=49:length(Estimations_RSSI.EPC)
    if Estimations_RSSI.Errors(k)>0
        s=s+Estimations_RSSI.Errors(k);
        num=num+1;
    end 
end
MeanError=s/num;
s1=0;
y1=0;
for k=49:length(Estimations_RSSI.EPC)
    if Estimations_RSSI.Errors(k)>0
        s1=s1+sqrt((Estimations_RSSI.Errors(k)-MeanError)^2);
        y1=y1+1;
    end 
end
std=sqrt(s1/y1);
fprintf('k : %d ||  dt : %d ||  C : %d ||  MeanError : %f || Std: %f  || Tags Found : %d \n',k_neighbors,dt,common_samples_threshold,MeanError,std,num);