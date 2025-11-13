import Service from "@/utils/request";

class Request extends Service{
	// 发送点击请求
	reqBrightScreen(body:any){
		return this.post({
			url:'/api/operphone',
			data:body
		})
	}
	reqEspStatus(){
		return this.get({
			url:'/api/esp-status'
		})
	}
}

export default new Request;