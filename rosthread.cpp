#include "rosthread.h"
#include <thread>



RosThread::RosThread(Filters *mFilters, pcl::PointCloud<PointT>::Ptr pcloud)
{
    this->mFilters = mFilters;
    this->pcloud = pcloud;
    subscrive_topics();
}

void RosThread::cloud_cb(const sensor_msgs::PointCloud2ConstPtr &cloud)
{
    //cout<<"Recieved pointcloud"<<endl;
    if ((cloud->width * cloud->height) == 0)
    {
        cout <<"Recieved empty point cloud"<<endl;
        return;
    }
    cout<<"Recieved cloud"<<endl;
    pcl::fromROSMsg(*cloud,*pcloud);
    mFilters->apply_filters();
}

bool RosThread::parameters_cb(alfa_msg::AlfaConfigure::Request &req, alfa_msg::AlfaConfigure::Response &res)
{
    alfa_msg::AlfaMetrics output;
    cout<<"Recieved FilterSettings with size" <<req.configurations.size()<<"... Updating"<<endl;
    for (int i=0; i< req.configurations.size();i++) {
        cout <<"Configuration: "<<i<< " With name: "<< req.configurations[i].config_name<< " with value: "<< req.configurations[i].config<<endl;
        alfa_msg::MetricMessage new_metric;
        new_metric.metric =  req.configurations[i].config;
        new_metric.metric_name = req.configurations[i].config_name;
        output.metrics.push_back(new_metric);
    }
    filter_metrics.publish(output);
    res.return_status=1;
    return true;
    //mFilters->update_filterSettings(msg);

}

void RosThread::init()
{
        char arg0[]= "filter_node";

        //strcpy(aux,output.toStdString().c_str());
        char *argv[]={arg0,NULL};
        int argc=(int)(sizeof(argv) / sizeof(char*)) - 1;;
        ros::init (argc, argv, "alfa_node");
          if (!ros::master::check()) {
              cout <<"Failed to inicialize ros"<<endl;
            return;
          }

}

void RosThread::subscrive_topics()
{
    sub_cloud = nh.subscribe("alfa_pointcloud",0,&RosThread::cloud_cb,this);


    sub_parameters = nh.advertiseService("alfa_filter_settings",&RosThread::parameters_cb,this);
    ros::NodeHandle n;
    filter_metrics = n.advertise<alfa_msg::AlfaMetrics>("alfa_filter_metrics", 1);
    m_spin_thread = new boost::thread(&RosThread::spin, this);

}

void RosThread::spin()
{
    int threads = std::thread::hardware_concurrency();
    std::cout << "Started Spinning with processor_count threads"<<threads << std::endl;
    ros::spin();
}
