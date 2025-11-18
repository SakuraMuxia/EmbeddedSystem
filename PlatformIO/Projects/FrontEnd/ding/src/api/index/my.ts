import Service from "@/utils/request2";

class Request extends Service{
	// 发送点击请求
	postCommand(action:any){
		return this.post({
			url:`/api/stream/${action}`
		})
	}
}
export default new Request;