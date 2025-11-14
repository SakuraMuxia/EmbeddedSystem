<template>
	<view class="content">
		<button @click="handleOpenScreen('unlock')" class="btn">解锁</button>
		<button @click="handleOpenScreen('esc')" class="btn">取消</button>
		<button @click="handleOpenScreen('enter')" class="btn">确认</button>
		<button @click="handleOpenScreen('rmtask')" class="btn">任务</button>
		<button @click="handleOpenScreen('right')" class="btn">右</button>
		<button @click="handleOpenScreen('down')" class="btn">下</button>
		<button @click="handleOpenScreen('tab')" class="btn">tab</button>
		<button @click="handleOpenScreen('menu')" class="btn">菜单</button>
		<button @click="handleOpenScreen('ding')" class="btn">相机</button>
		<button @click="handleOpenScreen('back')" class="btn">返回</button>
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
