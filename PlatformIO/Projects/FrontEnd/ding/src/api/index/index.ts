import Service from "@/utils/request";

class Request extends Service{
	// 发送点击请求
	postCommand(body:any){
		return this.post({
			url:'/api/operphone',
			data:body
		})
	}
	getEspAliveConnect(){
		return this.get({
			url:'/api/esp-status'
		})
	}
	postLogin(body:any){
		return this.post({
			url:'/api/login',
			data:body
		})
	}
	postLogout(body:any){
		return this.post({
			url:'/api/logout',
			data:body
		})
	}
}

export default new Request;