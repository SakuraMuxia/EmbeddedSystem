<template>
	<view class="content">
		<button @click="handleOpenScreen('unlock')" class="btn">解锁</button>
		<button @click="handleOpenScreen('2altTab')" class="btn">2altTab</button>
		<button @click="handleOpenScreen('switchDk9')" class="btn">切换到应用9</button>
		<button @click="handleOpenScreen('switchFh')" class="btn">返回应用首页</button>
		<button @click="handleOpenScreen('2esc')" class="btn">两次取消</button>
		<button @click="handleOpenScreen('esc')" class="btn">取消</button>
		<button @click="handleOpenScreen('enter')" class="btn">确认</button>
		<button @click="handleOpenScreen('clockIn')" class="btn">DK</button>
		<button @click="handleOpenScreen('tab')" class="btn">tab</button>
		<button @click="handleOpenScreen('menu')" class="btn">菜单</button>
		<button @click="handleOpenScreen('minimizeWindow')" class="btn">返回</button>
		<button @click="logout()" class="btn">退出登陆</button>
	</view>
</template>

<script setup lang="ts">
import { ref } from 'vue';
import indexReq from '@/api/index';

const title = ref('Hello');

const handleOpenScreen = async (oper:String) => {
	
	const body = {
		cmd:oper,
		deviceId:"esp32-0000_B853_70A5_0528"
	};
	await indexReq
		.postCommand(body)
		.then((res:any) => {
			console.log(res)
			// 消息提示
		})
		.catch((err:any) => {
			console.log(err)
			// 消息提示
		});
};

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
		
	}).catch(()=>{
		console.log()
	})
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
