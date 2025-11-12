import Service from "@/utils/request";

class Request extends Service{
	// 发送点击请求
	reqBrightScreen(body:any){
		return this.post({
			url:'/api/operphone',
			data:body
		})
	}
}

export default new Request;