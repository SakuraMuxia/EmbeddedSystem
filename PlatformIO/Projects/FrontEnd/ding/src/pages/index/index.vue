<template>
	<view class="content">
		<button @click="handleOpenScreen('unlock')" class="btn">解锁</button>
		<button @click="handleOpenScreen('2altTab')" class="btn">2altTab</button>
		<button @click="handleOpenScreen('switchDk9')" class="btn">切换到应用9</button>
		<button @click="handleOpenScreen('switchDk8')" class="btn">切换到应用8</button>
		<button @click="handleOpenScreen('2esc')" class="btn">两次取消</button>
		<button @click="handleOpenScreen('esc')" class="btn">取消</button>
		<button @click="handleOpenScreen('enter')" class="btn">确认</button>
		<button @click="handleOpenScreen('clockIn')" class="btn">DK</button>
		<button @click="handleOpenScreen('tab')" class="btn">tab</button>
		<button @click="handleOpenScreen('menu')" class="btn">菜单</button>
		<button @click="handleOpenScreen('minimizeWindow')" class="btn">返回</button>
		<button @click="disConnect('esp32-2805_A570_53B8')" class="btn">断开链接</button>
		<button @click="jumpToRealTime" class="btn">跳转实时</button>
		<button @click="logout()" class="btn">退出登陆</button>
		<button @click="startStream()" class="btn">开启推流</button>
		<button @click="stopStream()" class="btn">关闭推流</button>
	</view>
</template>

<script setup lang="ts">
import { ref } from 'vue';
import indexReq from '@/api/index';
import req from '@/api/index/my';
const title = ref('Hello');

const handleOpenScreen = async (oper:String) => {
	
	const body = {
		cmd:oper,
		deviceId:"esp32-2805_A570_53B8"
	};
	await indexReq
		.postCommand(body)
		.then((res:any) => {
			console.log(res)
			// 没有拦截状态
			if(res.success == true){
				// 消息提示
				uni.showToast({ title: '发送指令成功', icon: 'success' });
			}else{
				uni.showToast({ title: `发送指令失败${res.message}`, icon: 'fail' });
			}
			
		})
		.catch((err:any) => {
			console.log(err)
			// 消息提示
			uni.showToast({ title: `网络错误`, icon: 'fail' });
		});
};

const jumpToRealTime = () => {
	uni.reLaunch({
	  url: '/pages/my/my'
	});
}

const disConnect = async(id:any) => {
	await indexReq.getDisconnect(id).then((res:any)=>{
		// 没有拦截状态
		if(res.success == true){
			// 消息提示
			uni.showToast({ title: '发送指令成功', icon: 'success' });
		}else{
			uni.showToast({ title: `发送指令失败${res.message}`, icon: 'fail' });
		}
	}).catch((err:any)=>{
		uni.showToast({ title: '网络错误', icon: 'fail' });
	})
}

const logout = async() => {
	await indexReq.postLogout().then((res:any)=>{
		if(res.success == true){
			uni.setStorageSync('token', '');
			uni.showToast({ title: '退出成功', icon: 'success' });
			// 跳转页面
			uni.reLaunch({
				url: '/pages/login/login' // 登录成功后跳转首页
			});
		}else{
			uni.showToast({ title: '退出失败', icon: 'fail' });
		}
		
	}).catch((err)=>{
		console.log()
		
		uni.showToast({ title: `发送指令失败:${err}`, icon: 'fail' });
	})
}

const startStream = () => {
	req.postCommand('START').then((res)=>{
		if(res.success == true){
			
			uni.showToast({ title: '开启成功', icon: 'success' });
			// 跳转页面
			uni.reLaunch({
				url: '/pages/my/my' 
			});
		}else{
			uni.showToast({ title: '开启失败', icon: 'success' });
		}
	}).catch(()=>{
		
	});
}

const stopStream = () => {
	req.postCommand('STOP').then((res)=>{
		if(res.success == true){
			
			uni.showToast({ title: '关闭成功', icon: 'success' });
		}else{
			uni.showToast({ title: '关闭失败', icon: 'success' });
		}
	}).catch(()=>{
		
	});
}
</script>

<style lang="scss">
.content {
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;
	.btn{
		
		margin-bottom: 20rpx;
	}
}

.logo {
	height: 200rpx;
	width: 200rpx;
	margin-top: 200rpx;
	margin-left: auto;
	margin-right: auto;
	margin-bottom: 50rpx;
}

.text-area {
	display: flex;
	justify-content: center;
}

.title {
	font-size: 36rpx;
	color: #8f8f94;
}
</style>
