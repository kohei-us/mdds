#define MDDS_MTV_TRACE(method_type) ::mdds::detail::mtv::call_trace<Traits>{}({trace_method_t::method_type,this,__func__,"",__FILE__,__LINE__})

#define MDDS_MTV_TRACE_ARGS(method_type, stream) do{std::ostringstream _os_;_os_<<stream;::mdds::detail::mtv::call_trace<Traits>{}({trace_method_t::method_type,this,__func__,_os_.str(),__FILE__,__LINE__});}while (false)

