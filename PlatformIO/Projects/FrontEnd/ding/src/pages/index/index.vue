<template>
	<view class="content">
		<button @click="handleOpenScreen('click')" class="btn">亮屏</button>
		<button @click="handleOpenScreen('unlock')" class="btn">解锁</button>
		<button @click="handleOpenScreen('reset')" class="btn">复位</button>
		<button @click="handleOpenScreen('rmtask')" class="btn">清缓存</button>
		<button class="btn">发送回送日志</button>
		<button class="btn">叮一下</button>
	</view>
</template>

<script setup lang="ts">
import { ref } from 'vue';
import indexReq from '@/api/index';

const title = ref('Hello');

const handleOpenScreen = async (oper:String) => {
	console.log('操作');
	const data = {
		command:oper
	};
	await indexReq
		.reqBrightScreen(data)
		.then((res:any) => {
			console.log(res)
		})
		.catch((err:any) => {
			console.log(err)
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
